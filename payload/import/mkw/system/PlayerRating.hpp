#pragma once

#if RMC

namespace wwfc::mkw
{

class PlayerRating
{
public:
    constexpr PlayerRating()
    {
        m_points = 5000;
    }

    virtual ~PlayerRating();

    constexpr short GetPoints() const
    {
        return m_points;
    }

    constexpr void SetPoints(short points)
    {
        m_points = points;
    }

private:
    short m_points;
};

static_assert(sizeof(PlayerRating) == 0x8);

} // namespace wwfc::mkw

#endif