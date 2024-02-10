#include "ES.hpp"
#include <cstring>
#include <ogc/ipc.h>

typedef ioctlv IOVector;

s32 ES::s_fd = -1;

ES::ESError ES::Open()
{
    s_fd = IOS_Open("/dev/es", 0);

    return s_fd >= 0 ? ESError::OK : ESError(s_fd);
}

void ES::Close()
{
    if (s_fd >= 0) {
        IOS_Close(s_fd);
        s_fd = -1;
    }
}

ES::ESError ES::DIGetTicketView(Ticket* ticket, TicketView* view)
{
    if (s_fd < 0 || view == nullptr) {
        return ESError::INVALID;
    }

    Ticket tmpTicket alignas(32);
    if (ticket != nullptr) {
        std::memcpy(&tmpTicket, ticket, sizeof(ES::Ticket));
    }

    TicketView tmpView alignas(32);

    IOVector vec[4] alignas(32);
    vec[0].data = ticket != nullptr ? &tmpTicket : nullptr;
    vec[0].len = ticket != nullptr ? sizeof(ES::Ticket) : 0;
    vec[1].data = &tmpView;
    vec[1].len = sizeof(ES::TicketView);

    s32 ret = IOS_Ioctlv(s_fd, s32(ESIoctl::DI_GET_TICKET_VIEW), 1, 1, vec);

    std::memcpy(view, &tmpView, sizeof(ES::TicketView));
    return ESError(ret);
}