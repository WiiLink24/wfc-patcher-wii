#include "ES.hpp"
#include "IOS.hpp"
#include "Util.hpp"
#include <cstring>
#include <ogc/ipc.h>

s32 ES::s_fd = -1;

/**
 * Open the ES interface.
 * @return ESError::OK on success, otherwise an error code.
 */
ES::ESError ES::Open()
{
    s_fd = IOS_Open("/dev/es", 0);

    return s_fd >= 0 ? ESError::OK : ESError(s_fd);
}

/**
 * Close the ES interface.
 */
void ES::Close()
{
    if (s_fd >= 0) {
        IOS_Close(s_fd);
        s_fd = -1;
    }
}

/**
 * Reset the ES interface without calling IOS_Close, usually used for IOS
 * reload.
 */
void ES::Reset()
{
    s_fd = -1;
}

/**
 * ES_LaunchTitle; Launch a title using IOS_IoctlvReboot.
 * @param[in] titleId Title ID to launch.
 * @param[in] view Optional ticket view to use for the launch. The function
 * will get the ticket view itself if null.
 */
ES::ESError ES::LaunchTitleReboot(u64 titleId, const TicketView* view)
{
    if (s_fd < 0) {
        return ESError::INVALID;
    }

    ES::TicketView drvView alignas(32) = {};
    if (view != nullptr) {
        std::memcpy(&drvView, view, sizeof(ES::TicketView));
    } else {
        // Get the ticket view ourselves
        ESError ret = GetTicketViews(titleId, 1, &drvView);
        if (ret != ESError::OK) {
            return ret;
        }
    }

    u64 drvTitleId[4] alignas(32) = {titleId};

    IOS::IOVector vec[4] alignas(32) = {};
    vec[0].data = drvTitleId;
    vec[0].len = sizeof(u64);
    vec[1].data = &drvView;
    vec[1].len = sizeof(ES::TicketView);

    s32 ret = IOS_IoctlvReboot(s_fd, s32(ESIoctl::LAUNCH_TITLE), 1, 1, vec);
    if (ret < 0) {
        return static_cast<ESError>(ret);
    }

    Reset();
    return static_cast<ESError>(ret);
}

/**
 * ES_GetNumTicketViews; Get the number of ticket views for a title.
 * @param[in] titleID Title ID to get the number of ticket views for.
 * @param[out] outCount Output number of ticket views.
 */
ES::ESError ES::GetNumTicketViews(u64 titleId, u32* outCount)
{
    if (s_fd < 0 || outCount == nullptr) {
        return ESError::INVALID;
    }

    u64 drvTitleId[4] alignas(32) = {titleId};
    u32 drvCount[8] alignas(32) = {};

    IOS::IOVector vec[4] alignas(32) = {};
    vec[0].data = drvTitleId;
    vec[0].len = sizeof(u64);
    vec[1].data = drvCount;
    vec[1].len = sizeof(u32);

    s32 ret = IOS_Ioctlv(s_fd, s32(ESIoctl::GET_NUM_TICKET_VIEWS), 1, 1, vec);
    *outCount = drvCount[0];
    return static_cast<ESError>(ret);
}

/**
 * ES_GetTicketViews; Get the ticket views for a title.
 * @param[in] titleId Title ID to get the ticket views for.
 * @param[in] count Number of ticket views to get.
 * @param[out] outViews Output ticket views.
 */
ES::ESError ES::GetTicketViews(u64 titleId, u32 count, TicketView* outViews)
{
    if (s_fd < 0 || outViews == nullptr) {
        return ESError::INVALID;
    }

    u64 drvTitleId[4] alignas(32) = {titleId};
    u32 drvCount[8] alignas(32) = {count};

    IOS::IOVector vec[4] alignas(32) = {};
    vec[0].data = drvTitleId;
    vec[0].len = sizeof(u64);
    vec[1].data = drvCount;
    vec[1].len = sizeof(u32);
    vec[2].data = reinterpret_cast<void*>(outViews);
    vec[2].len = sizeof(TicketView) * count;

    s32 ret = IOS_Ioctlv(s_fd, s32(ESIoctl::GET_TICKET_VIEWS), 2, 1, vec);
    return static_cast<ESError>(ret);
}

/**
 * ES_DiGetTicketView; Get the ticket view of the current title context.
 * @param[in] ticket Optional ticket to use for the view.
 * @param[out] view Output ticket view.
 */
ES::ESError ES::DIGetTicketView(Ticket* ticket, TicketView* outView)
{
    if (s_fd < 0 || outView == nullptr) {
        return ESError::INVALID;
    }

    Ticket tmpTicket alignas(32);
    if (ticket != nullptr) {
        std::memcpy(&tmpTicket, ticket, sizeof(ES::Ticket));
    }

    TicketView drvView alignas(32) = {};

    IOS::IOVector vec[4] alignas(32) = {};
    vec[0].data = ticket != nullptr ? &tmpTicket : nullptr;
    vec[0].len = ticket != nullptr ? sizeof(ES::Ticket) : 0;
    vec[1].data = &drvView;
    vec[1].len = sizeof(ES::TicketView);

    s32 ret = IOS_Ioctlv(s_fd, s32(ESIoctl::DI_GET_TICKET_VIEW), 1, 1, vec);

    std::memcpy(outView, &drvView, sizeof(ES::TicketView));
    return static_cast<ESError>(ret);
}