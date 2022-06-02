// Copyright 2022 Robert Ryan. See Licence.md.

#version 460 core

layout(location = 0) out vec4 outCol;
layout(location = 0) in vec4 inCol;

void main()
{
    outCol = inCol;
}
