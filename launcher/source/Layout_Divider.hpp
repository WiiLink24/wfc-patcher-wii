#pragma once

#include "Layout.hpp"

class Layout_Divider : public Layout
{
public:
    void Init();
    void Calc() override;
    void Draw() override;
};