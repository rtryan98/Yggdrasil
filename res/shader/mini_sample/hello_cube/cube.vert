// Copyright 2022 Robert Ryan. See Licence.md.

#version 460 core

layout(location = 0) out vec4 outCol;

struct Vertex
{
    float x, y, z;
    uint col;
};

layout(set = 0, binding = 0, std430) readonly restrict buffer V { Vertex[] verts; };
layout(set = 0, binding = 1, std140) readonly restrict uniform U { mat4 transform; };

void main()
{
    Vertex vert = verts[gl_VertexIndex];
    outCol = unpackUnorm4x8(vert.col);
    gl_Position = transform * vec4(vert.x, vert.y, vert.z, 1.0);
}
