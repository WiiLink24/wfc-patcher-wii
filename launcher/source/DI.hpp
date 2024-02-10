#pragma once

#include "ES.hpp"
#include <gctypes.h>

// Documentation of the drive used here is referenced from
// - https://wiibrew.org/wiki//dev/di
// - Yet Another GameCube Documentation

class DI
{
public:
    enum class DIError : s32 {
        UNKNOWN = 0x0,
        OK = 0x1,
        DRIVE = 0x2,
        COVER_CLOSED = 0x4,
        TIMEOUT = 0x10,
        SECURITY = 0x20,
        VERIFY = 0x40,
        INVALID = 0x80,
    };

    enum class DIIoctl : u8 {
        INQUIRY = 0x12,
        READ_DISK_ID = 0x70,
        READ = 0x71,
        WAIT_FOR_COVER_CLOSE = 0x79,
        GET_COVER_REGISTER = 0x7A,
        NOTIFY_RESET = 0x7E,
        SET_SPINUP_FLAG = 0x7F,
        READ_DVD_PHYSICAL = 0x80,
        READ_DVD_COPYRIGHT = 0x81,
        READ_DVD_DISC_KEY = 0x82,
        GET_LENGTH = 0x83,
        GET_DIMMBUF = 0x84,
        MASK_COVER_INTERRUPT = 0x85,
        CLEAR_COVER_INTERRUPT = 0x86,
        UNMASK_STATUS_INTERRUPTS = 0x87,
        GET_COVER_STATUS = 0x88,
        UNMASK_COVER_INTERRUPT = 0x89,
        RESET = 0x8A,
        OPEN_PARTITION = 0x8B,
        CLOSE_PARTITION = 0x8C,
        UNENCRYPTED_READ = 0x8D,
        ENABLE_DVD_VIDEO = 0x8E,
        GET_NO_DISC_OPEN_PARTITION_PARAMS = 0x90,
        NO_DISC_OPEN_PARTITION = 0x91,
        GET_NO_DISC_BUFFER_SIZES = 0x92,
        OPEN_PARTITION_WITH_TMD_AND_TICKET = 0x93,
        OPEN_PARTITION_WITH_TMD_AND_TICKET_VIEW = 0x94,
        GET_STATUS_REGISTER = 0x95,
        GET_CONTROL_REGISTER = 0x96,
        REPORT_KEY = 0xA4,
        SEEK = 0xAB,
        READ_DVD = 0xD0,
        READ_DVD_CONFIG = 0xD1,
        STOP_LASER = 0xD2,
        OFFSET = 0xD9,
        READ_DISK_BCA = 0xDA,
        REQUEST_DISC_STATUS = 0xDB,
        REQUEST_RETRY_NUMBER = 0xDC,
        SET_MAXIMUM_ROTATION = 0xDD,
        SER_MEAS_CONTROL = 0xDF,
        REQUEST_ERROR = 0xE0,
        AUDIO_STREAM = 0xE1,
        REQUEST_AUDIO_STATUS = 0xE2,
        STOP_MOTOR = 0xE3,
        AUDIO_BUFFER_CONFIG = 0xE4,
    };

    struct DriveInfo {
        u16 revisionLevel;
        u16 deviceCode;
        u32 releaseDate;
        u8 version; // ?
        u8 pad[0x17];
    };

    struct DiskID {
        char gameId[4];
        u16 groupId;
        u8 discNum;
        u8 discVer;
        u8 discStreamFlag;
        u8 discStreamSize;
        u8 pad[0xE];
        u32 discMagic;
        u32 discMagicGC;
    };

    struct DICommand {
        DIIoctl cmd;
        // implicit pad
        u32 args[7];
    };

    static_assert(sizeof(DICommand) == 0x20);

    struct Partition {
        ES::Ticket ticket;
        u32 tmdByteLength;
        u32 tmdWordOffset;
        u32 certChainByteLength;
        u32 certChainWordOffset;
        u32 h3TableWordOffset;
        u32 dataWordOffset;
        u32 dataWordLength;
    };

    static_assert(sizeof(DICommand) == 0x20);

    static bool Init();

    /**
     * DVDLowInquiry; Retrieves information about the drive version.
     */
    static DIError Inquiry(DriveInfo* info);

    /**
     * DVDLowReadDiskID; Reads the current disc ID and initializes the drive.
     */
    static DIError ReadDiskID(DiskID* diskId);

    /**
     * DVDLowRead; Reads and decrypts disc data. This command can only be used
     * if hashing and encryption are enabled for the disc. DVDLowOpenPartition
     * needs to have been called before for the keys to be read.
     */
    static DIError Read(void* data, u32 lenBytes, u32 wordOffset);

    /**
     * DVDLowWaitForCoverClose; Waits for a disc to be inserted; if there is
     * already a disc inserted, it must be removed first. This command does not
     * time out; if no disc is inserted, it will wait forever. Returns
     * DIError::CoverClosed on success.
     */
    static DIError WaitForCoverClose();

    /**
     * DVDLowGetLength; Get the length of the last transfer.
     * @param[out] length Output length.
     */
    static DIError GetLength(u32* length);

    /**
     * DVDLowReset; Resets the disc drive.
     * @param[in] spinup Set to true to spinup the drive once a disc is
     * inserted, or false to stop the drive.
     */
    static DIError Reset(bool spinup);

    /**
     * DVDLowOpenPartition; Opens a partition, including verifying it through
     * ES. ReadDiskID needs to have been called beforehand.
     * @param[out] tmd Output title metadata (required, must be 32 byte aligned)
     * @param[in] ticket Input ticket (can be nullptr, must be 32 byte aligned)
     * @param[in] certs Input certificate chain (can be nullptr, must be 32 byte
     * aligned)
     */
    static DIError OpenPartition(
        u32 wordOffset, ES::TMDFixed<512>* tmd, ES::ESError* esError = nullptr,
        const ES::Ticket* ticket = nullptr, const void* certs = nullptr,
        u32 certsLen = 0
    );

    /**
     * DVDLowClosePartition; Closes the currently-open partition, removing
     * information about its keys and such.
     */
    static DIError ClosePartition();

    /**
     * DVDLowUnencryptedRead; Reads raw data from the disc. Only usable in the
     * "System Area" of the disc.
     */
    static DIError UnencryptedRead(void* data, u32 lenBytes, u32 wordOffset);

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
    static DIError OpenPartitionWithTmdAndTicket(
        u32 wordOffset, ES::TMD* tmd, ES::ESError* esError = nullptr,
        const ES::Ticket* ticket = nullptr, const void* certs = nullptr,
        u32 certsLen = 0
    );

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
    static DIError OpenPartitionWithTmdAndTicketView(
        u32 wordOffset, ES::TMD* tmd, ES::ESError* esError = nullptr,
        const ES::TicketView* ticketView = nullptr, const void* certs = nullptr,
        u32 certsLen = 0
    );

    /**
     * DVDLowSeek; Seeks to the sector containing a specific position on the
     * disc.
     */
    static DIError Seek(u32 wordOffset);

    /**
     * DVDLowReadDiskBca; Reads the last 64 bytes of the BCA (burst cutting
     * area).
     */
    static DIError ReadDiskBCA(u8* out);

private:
    static DIError CallIoctl(
        DICommand& block, DIIoctl cmd, void* out = nullptr, u32 outLen = 0
    );

    static s32 s_fd;
};