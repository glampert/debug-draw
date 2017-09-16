
// ================================================================================================
// -*- C++ -*-
// File:   sample_null_renderer.cpp
// Author: Guilherme R. Lampert
// Brief:  Minimal Debug Draw usage sample that does nothing (a null renderer).
//
// This software is in the public domain. Where that dedication is not recognized,
// you are granted a perpetual, irrevocable license to copy, distribute, and modify
// this file as you see fit.
// ================================================================================================

#define DEBUG_DRAW_IMPLEMENTATION
#include "debug_draw.hpp"

class DDRenderInterfaceNull final
    : public dd::RenderInterface
{
public:
    ~DDRenderInterfaceNull() { }
};

int main()
{
    DDRenderInterfaceNull ddRenderIfaceNull;

    dd::initialize(&ddRenderIfaceNull);

    dd::flush();

    dd::shutdown();
}

