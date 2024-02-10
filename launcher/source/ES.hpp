#pragma once

#include <gctypes.h>
#include <gcutil.h>

class ES
{
public:
    enum class ESError : s32 {
        OK = 0,
        INVALID_PUB_KEY_TYPE = -1005,
        READ_ERROR = -1009,
        WRITE_ERROR = -1010,
        INVALID_SIG_TYPE = -1012,
        MAX_OPEN = -1016,
        INVALID = -1017,
        DEVICE_ID_MATCH = -1020,
        HASH_MATCH = -1022,
        NO_MEMORY = -1024,
        NO_ACCESS = -1026,
        ISSUER_NOT_FOUND = -1027,
        TICKET_NOT_FOUND = -1028,
        INVALID_TICKET = -1029,
        OUTDATED_BOOT2 = -1031,
        TICKET_LIMIT = -1033,
        OUTDATED_TITLE = -1035,
        REQUIRED_IOS_NOT_INSTALLED = -1036,
        WRONG_TMD_CONTENT_COUNT = -1037,
        NO_TMD = -1039,
    };

    enum class ESIoctl {
        ADD_TICKET = 0x01,
        ADD_TITLE_START = 0x02,
        ADD_CONTENT_START = 0x03,
        ADD_CONTENT_DATA = 0x04,
        ADD_CONTENT_FINISH = 0x05,
        ADD_TITLE_FINISH = 0x06,
        GET_DEVICE_ID = 0x07,
        LAUNCH_TITLE = 0x08,
        OPEN_CONTENT = 0x09,
        READ_CONTENT = 0x0A,
        CLOSE_CONTENT = 0x0B,
        GET_OWNED_TITLES_COUNT = 0x0C,
        GET_OWNED_TITLES = 0x0D,
        GET_TITLES_COUNT = 0x0E,
        GET_TITLES = 0x0F,
        GET_TITLE_CONTENTS_COUNT = 0x10,
        GET_TITLE_CONTENTS = 0x11,
        GET_NUM_TICKET_VIEWS = 0x12,
        GET_TICKET_VIEWS = 0x13,
        GET_TMD_VIEW_SIZE = 0x14,
        GET_TMD_VIEW = 0x15,
        GET_CONSUMPTION = 0x16,
        DELETE_TITLE = 0x17,
        DELETE_TICKET = 0x18,
        DI_GET_TMD_VIEW_SIZE = 0x19,
        DI_GET_TMD_VIEW = 0x1A,
        DI_GET_TICKET_VIEW = 0x1B,
        DI_VERIFY = 0x1C,
        GET_DATA_DIR = 0x1D,
        GET_DEVICE_CERT = 0x1E,
        IMPORT_BOOT = 0x1F,
        GET_TITLE_ID = 0x20,
        SET_UID = 0x21,
        DELETE_TITLE_CONTENT = 0x22,
        SEEK_CONTENT = 0x23,
        OPEN_TITLE_CONTENT = 0x24,
        LAUNCH_BC = 0x25,
        EXPORT_TITLE_INIT = 0x26,
        EXPORT_CONTENT_BEGIN = 0x27,
        EXPORT_CONTENT_DATA = 0x28,
        EXPORT_CONTENT_END = 0x29,
        EXPORT_TITLE_DONE = 0x2A,
        ADD_TMD = 0x2B,
        ENCRYPT = 0x2C,
        DECRYPT = 0x2D,
        GET_BOOT2_VERSION = 0x2E,
        ADD_TITLE_CANCEL = 0x2F,
        SIGN = 0x30,
        VERIFY_SIGN = 0x31,
        GET_STORED_CONTENT_COUNT = 0x32,
        GET_STORED_CONTENT = 0x33,
        GET_STORED_TMD_SIZE = 0x34,
        GET_STORED_TMD = 0x35,
        GET_SHARED_CONTENT_COUNT = 0x36,
        GET_SHARED_CONTENTS = 0x37,
        DELETE_SHARED_CONTENT = 0x38,
        GET_DI_TMD_SIZE = 0x39,
        GET_DI_TMD = 0x3A,
        DI_VERIFY_WITH_TICKET_VIEW = 0x3B,
        SETUP_STREAM_KEY = 0x3C,
        DELETE_STREAM_KEY = 0x3D,
    };

    enum class SigType : u32 {
        RSA_2048 = 0x00010001,
        RSA_4096 = 0x00010000,
    };

    enum class Region : u16 {
        JAPAN = 0,
        USA = 1,
        EUROPE = 2,
        NONE = 3,
        KOREA = 4,
    };

    enum AccessFlag {
        ACCESS_FLAG_HARDWARE = 0x1,
        ACCESS_FLAG_DVDVIDEO = 0x2,
    };

    struct TMDContent {
        enum Type : u16 {
            NORMAL = 0x0001,
            DLC = 0x4001,
            SHARED = 0x8001,
        };

        u32 cid;
        u16 index;
        Type type;
        u64 size;
        u8 hash[0x14];
    } ATTRIBUTE_PACKED;

    struct TMDHeader {
        SigType sigType;
        u8 sigBlock[256];
        u8 fill1[60];
        char issuer[64];
        u8 version;
        u8 caCRLVersion;
        u8 signerCRLVersion;
        u8 vWiiTitle;
        u64 iosTitleId;
        u64 titleId;
        u32 titleType;
        u16 groupId;
        u16 zero;
        Region region;
        u8 ratings[16];
        u8 reserved[12];
        u8 ipcMask[12];
        u8 reserved2[18];
        u32 accessRights;
        u16 titleVersion;
        u16 numContents;
        u16 bootIndex;
        u16 fill2;
    } ATTRIBUTE_PACKED;

    static_assert(sizeof(TMDHeader) == 0x1E4);

    struct TMD : TMDHeader {
        TMDContent* GetContents()
        {
            return reinterpret_cast<TMDContent*>(this + 1);
        }

        u32 GetSize() const
        {
            return sizeof(TMDHeader) + sizeof(TMDContent) * numContents;
        }
    } ATTRIBUTE_PACKED;

    template <u16 TNumContents>
    struct TMDFixed : TMD {
        TMDContent contents[TNumContents];

        u32 GetSize() const
        {
            return sizeof(TMDHeader) + sizeof(TMDContent) * TNumContents;
        }
    } ATTRIBUTE_PACKED;

    typedef TMDFixed<512> TMDFull;

    struct TicketLimit {
        u32 tag;
        u32 value;
    } ATTRIBUTE_PACKED;

    static_assert(sizeof(TicketLimit) == 0x8);

    struct TicketInfo {
        u64 ticketID;
        u32 consoleID;
        u64 titleID;
        u16 unknown_0x1E4;
        u16 ticketTitleVersion;
        u16 permittedTitlesMask;
        u32 permitMask;
        bool allowTitleExport;
        u8 commonKeyIndex;
        u8 reserved[0x30];
        u8 cidxMask[0x40];
        u16 fill4;
        TicketLimit limits[8];
        u16 fill8;
    } ATTRIBUTE_PACKED;

    static_assert(sizeof(TicketInfo) == 0xD4);

    struct Ticket {
        SigType sigType;
        u8 sigBlock[0x100];
        u8 fill1[0x3C];
        char issuer[64];
        u8 fill2[0x3F];
        u8 titleKey[16];
        u8 fill3;
        TicketInfo info;
    } ATTRIBUTE_PACKED;

    static_assert(sizeof(Ticket) == 0x2A4);

    struct TicketView {
        u32 view;
        TicketInfo info;
    } ATTRIBUTE_PACKED;

    static_assert(sizeof(TicketView) == 0xD8);

    static ESError Open();
    static void Close();

    /**
     * ES_DiGetTicketView; Get the ticket view of the current title context.
     * @param[in] ticket Optional ticket to use for the view.
     * @param[out] view Output ticket view.
     */
    static ESError DIGetTicketView(ES::Ticket* ticket, ES::TicketView* view);

private:
    static s32 s_fd;
};