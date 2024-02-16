#pragma once

#include "Layout.hpp"

class Layout_Logo : public Layout
{
public:
    void Init();
    void Calc() override;
    void Draw() override;
};