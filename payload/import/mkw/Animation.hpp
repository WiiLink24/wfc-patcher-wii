#pragma once

#if RMC

#  include <wwfcTypes.h>

// https://github.com/MelgMKW/Pulsar/blob/main/GameSource/MarioKartWii/3D/Model/AnmHolder.hpp
namespace wwfc::mkw::Model
{

class Animation
{
public:
    virtual void vf_00();
    virtual void vf_04();
    virtual float GetFrameCount() const;
    virtual void vf_0C();
    virtual void vf_10();
    virtual void vf_14();
    virtual void vf_18();
    virtual void vf_1C();
    virtual void vf_20();

private:
    /* 0x04 */ u8 _04[0x1C - 0x04];
};

static_assert(sizeof(Animation) == 0x1C);

class AnimationTexturePattern : public Animation
{
public:
    float GetFrameCount() const override;

private:
    /* 0x1C */ u8 _1C[0x20 - 0x1C];
};

static_assert(sizeof(AnimationTexturePattern) == 0x20);

} // namespace wwfc::mkw::Model

#endif // RMC
