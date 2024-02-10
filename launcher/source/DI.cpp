#include "DI.hpp"
#include "Util.hpp"
#include <ogc/ipc.h>

typedef ioctlv IOVector;

s32 DI::s_fd = -1;

bool DI::Init()
{
    s_fd = IOS_Open("/dev/di", 0);

    return s_fd >= 0;
}

/**
 * DVDLowInquiry; Retrieves information about the drive version.
 */
DI::DIError DI::Inquiry(DriveInfo* info)
{
    DICommand block = {
        .cmd = DIIoctl::INQUIRY,
        .args = {0},
    };
    DriveInfo drvInfo alignas(32);

    DIError res =
        CallIoctl(block, DIIoctl::INQUIRY, &drvInfo, sizeof(DriveInfo));
    if (res == DIError::OK) {
        *info = drvInfo;
    }
    return res;
}

/**
 * DVDLowReadDiskID; Reads the current disc ID and initializes the drive.
 */
DI::DIError DI::ReadDiskID(DiskID* diskId)
{
    DICommand block = {
        .cmd = DIIoctl::READ_DISK_ID,
        .args = {0},
    };
    DiskID drvDiskid alignas(32);

    DIError res =
        CallIoctl(block, DIIoctl::READ_DISK_ID, &drvDiskid, sizeof(DiskID));
    if (res == DIError::OK) {
        *diskId = drvDiskid;
    }

    return res;
}

/**
 * DVDLowRead; Reads and decrypts disc data. This command can only be used
 * if hashing and encryption are enabled for the disc. DVDLowOpenPartition
 * needs to have been called before for the keys to be read.
 */
DI::DIError DI::Read(void* data, u32 lenBytes, u32 wordOffset)
{
    DICommand block = {
        .cmd = DIIoctl::READ,
        .args = {lenBytes, wordOffset},
    };
    return CallIoctl(block, DIIoctl::READ, data, lenBytes);
}

/**
 * DVDLowWaitForCoverClose; Waits for a disc to be inserted; if there is
 * already a disc inserted, it must be removed first. This command does not
 * time out; if no disc is inserted, it will wait forever. Returns
 * DIError::CoverClosed on success.
 */
DI::DIError DI::WaitForCoverClose()
{
    DICommand block = {
        .cmd = DIIoctl::WAIT_FOR_COVER_CLOSE,
        .args = {0},
    };
    return CallIoctl(block, DIIoctl::WAIT_FOR_COVER_CLOSE);
}

/**
 * DVDLowGetLength; Get the length of the last transfer.
 * @param[out] length Output length.
 */
DI::DIError DI::GetLength(u32* length)
{
    DICommand block = {
        .cmd = DIIoctl::GET_LENGTH,
        .args = {0},
    };
    u32 drvLength;
    DIError res =
        CallIoctl(block, DIIoctl::GET_LENGTH, &drvLength, sizeof(u32));
    *length = drvLength;
    return res;
}

/**
 * DVDLowReset; Resets the disc drive.
 * @param[in] spinup Set to true to spinup the drive once a disc is
 * inserted, or false to stop the drive.
 */
DI::DIError DI::Reset(bool spinup)
{
    DICommand block = {
        .cmd = DIIoctl::RESET,
        .args = {static_cast<u32>(spinup)},
    };
    return CallIoctl(block, DIIoctl::RESET);
}

/**
 * DVDLowOpenPartition; Opens a partition, including verifying it through
 * ES. ReadDiskID needs to have been called beforehand.
 * @param[out] tmd Output title metadata (required, must be 32 byte aligned)
 * @param[in] ticket Input ticket (can be nullptr, must be 32 byte aligned)
 * @param[in] certs Input certificate chain (can be nullptr, must be 32 byte
 * aligned)
 */
DI::DIError DI::OpenPartition(
    u32 wordOffset, ES::TMDFixed<512>* tmd, ES::ESError* esError,
    const ES::Ticket* ticket, const void* certs, u32 certsLen
)
{
    if (!IsAligned(tmd, 32) || !IsAligned(ticket, 32) ||
        !IsAligned(certs, 32)) {
        return DIError::INVALID;
    }

    DICommand block = {
        .cmd = DIIoctl::OPEN_PARTITION,
        .args = {wordOffset},
    };
    u32 output[8] alignas(32);

    IOVector vec[8] alignas(32);
    // input - Command block
    vec[0].data = &block;
    vec[0].len = sizeof(DICommand);
    // input - Ticket (optional)
    vec[1].data = const_cast<ES::Ticket*>(ticket);
    vec[1].len = ticket ? sizeof(ES::Ticket) : 0;
    // input - Shared certs (optional)
    vec[2].data = const_cast<void*>(certs);
    vec[2].len = certs ? certsLen : 0;
    // output - TMD
    vec[3].data = tmd;
    vec[3].len = sizeof(ES::TMDFixed<512>);
    // output - ES Error
    vec[4].data = output;
    vec[4].len = sizeof(output);

    DIError res = static_cast<DIError>(
        IOS_Ioctlv(s_fd, s32(DIIoctl::OPEN_PARTITION), 3, 2, vec)
    );
    if (esError != nullptr) {
        *esError = static_cast<ES::ESError>(output[0]);
    }

    return res;
}

/**
 * DVDLowClosePartition; Closes the currently-open partition, removing
 * information about its keys and such.
 */
DI::DIError DI::ClosePartition()
{
    DICommand block = {
        .cmd = DIIoctl::CLOSE_PARTITION,
        .args = {0},
    };
    return CallIoctl(block, DIIoctl::CLOSE_PARTITION);
}

/**
 * DVDLowUnencryptedRead; Reads raw data from the disc. Only usable in the
 * "System Area" of the disc.
 */
DI::DIError DI::UnencryptedRead(void* data, u32 lenBytes, u32 wordOffset)
{
    DICommand block = {
        .cmd = DIIoctl::UNENCRYPTED_READ, .args = {lenBytes, wordOffset}};
    return CallIoctl(block, DIIoctl::UNENCRYPTED_READ, data, lenBytes);
}

/**
 * DVDLowOpenPartitionWithTmdAndTicket; Opens a partition, including
 * verifying it through ES. ReadDiskID needs to have been called beforehand.
 * This function takes an already-read TMD and can take an already-read
 * ticket, which means it can be faster since the ticket does not need to be
 * read from the disc.
 * @param[in] tmd Input title metadata (required, must be 32 byte aligned)
 * @param[in] ticket Input ticket (can be nullptr, must be 32 byte aligned)
 * @param[in] certs Input certificate chain (can be nullptr, must be 32 byte
 * aligned)
 */
DI::DIError DI::OpenPartitionWithTmdAndTicket(
    u32 wordOffset, ES::TMD* tmd, ES::ESError* esError,
    const ES::Ticket* ticket, const void* certs, u32 certsLen
)
{
    if (!IsAligned(tmd, 32) || !IsAligned(ticket, 32) ||
        !IsAligned(certs, 32)) {
        return DIError::INVALID;
    }

    DICommand block = {
        .cmd = DIIoctl::OPEN_PARTITION_WITH_TMD_AND_TICKET,
        .args = {wordOffset},
    };
    u32 output[8] alignas(32);

    IOVector vec[8] alignas(32);
    // input - Command block
    vec[0].data = &block;
    vec[0].len = sizeof(DICommand);
    // input - Ticket (optional)
    vec[1].data = const_cast<ES::Ticket*>(ticket);
    vec[1].len = ticket ? sizeof(ES::Ticket) : 0;
    // input - TMD
    vec[2].data = tmd;
    vec[2].len = tmd->GetSize();
    // input - Shared certs (optional)
    vec[3].data = const_cast<void*>(certs);
    vec[3].len = certs ? certsLen : 0;
    // output - ES Error
    vec[4].data = output;
    vec[4].len = sizeof(output);

    DIError res = static_cast<DIError>(IOS_Ioctlv(
        s_fd, s32(DIIoctl::OPEN_PARTITION_WITH_TMD_AND_TICKET), 4, 1, vec
    ));
    if (esError != nullptr) {
        *esError = static_cast<ES::ESError>(output[0]);
    }

    return res;
}

/**
 * DVDLowOpenPartitionWithTmdAndTicketView; Opens a partition, including
 * verifying it through ES. ReadDiskID needs to have been called beforehand.
 * This function takes an already-read TMD and can take an already-read
 * ticket view, which means it can be faster since the ticket does not need
 * to be read from the disc.
 * @param[in] tmd Input title metadata (required, must be 32 byte aligned)
 * @param[in] ticketView Input ticket view (can be nullptr, must be 32 byte
 * aligned)
 * @param[in] certs Input certificate chain (can be nullptr, must be 32 byte
 * aligned)
 */
DI::DIError DI::OpenPartitionWithTmdAndTicketView(
    u32 wordOffset, ES::TMD* tmd, ES::ESError* esError,
    const ES::TicketView* ticketView, const void* certs, u32 certsLen
)
{
    if (!IsAligned(tmd, 32) || !IsAligned(ticketView, 32) ||
        !IsAligned(certs, 32)) {
        return DIError::INVALID;
    }

    DICommand block = {
        .cmd = DIIoctl::OPEN_PARTITION_WITH_TMD_AND_TICKET_VIEW,
        .args = {wordOffset},
    };
    u32 output[8] alignas(32);

    IOVector vec[8] alignas(32);
    // input - Command block
    vec[0].data = &block;
    vec[0].len = sizeof(DICommand);
    // input - Ticket View (optional)
    vec[1].data = const_cast<ES::TicketView*>(ticketView);
    vec[1].len = ticketView ? sizeof(ES::TicketView) : 0;
    // input - TMD
    vec[2].data = tmd;
    vec[2].len = tmd->GetSize();
    // input - Shared certs (optional)
    vec[3].data = const_cast<void*>(certs);
    vec[3].len = certs ? certsLen : 0;
    // output - ES Error
    vec[4].data = output;
    vec[4].len = sizeof(output);

    DIError res = static_cast<DIError>(IOS_Ioctlv(
        s_fd, s32(DIIoctl::OPEN_PARTITION_WITH_TMD_AND_TICKET_VIEW), 4, 1, vec
    ));
    if (esError != nullptr) {
        *esError = static_cast<ES::ESError>(output[0]);
    }

    return res;
}

/**
 * DVDLowSeek; Seeks to the sector containing a specific position on the
 * disc.
 */
DI::DIError DI::Seek(u32 wordOffset)
{
    DICommand block = {
        .cmd = DIIoctl::SEEK,
        .args = {wordOffset},
    };
    return CallIoctl(block, DIIoctl::SEEK);
}

/**
 * DVDLowReadDiskBca; Reads the last 64 bytes of the BCA (burst cutting
 * area).
 */
DI::DIError DI::ReadDiskBCA(u8* out)
{
    if (!IsAligned(out, 32)) {
        return DIError::INVALID;
    }

    DICommand block = {
        .cmd = DIIoctl::READ_DISK_BCA,
        .args = {0},
    };
    return CallIoctl(block, DIIoctl::READ_DISK_BCA, out, 64);
}

DI::DIError DI::CallIoctl(DICommand& block, DIIoctl cmd, void* out, u32 outLen)
{
    return static_cast<DIError>(
        IOS_Ioctl(s_fd, u32(cmd), &block, sizeof(DICommand), out, outLen)
    );
}