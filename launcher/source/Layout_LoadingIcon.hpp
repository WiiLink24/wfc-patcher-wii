#pragma once

#include "Layout.hpp"

class Layout_LoadingIcon : public Layout
{
public:
    void Init();
    void Calc() override;
    void Draw() override;

private:
    u32 m_frame;
};