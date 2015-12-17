
// ================================================================================================
// -*- C++ -*-
// File: sample_null_renderer.cpp
// Author: Guilherme R. Lampert
// Created on: 17/12/15
// Brief: Minimal Debug Draw usage sample that does nothing (a null renderer).
// ================================================================================================

#define DEBUG_DRAW_IMPLEMENTATION
#include "debug_draw.hpp"

class DDRenderInterfaceNull
    : public dd::RenderInterface
{
public:
    ~DDRenderInterfaceNull() { }
};

int main()
{
    DDRenderInterfaceNull ddRenderIfaceNull;

    dd::initialize(&ddRenderIfaceNull);

    dd::flush(0);

    dd::shutdown();
}
