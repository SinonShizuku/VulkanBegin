#version 460
#pragma shader_stage(fragment)

layout(location = 0) in vec4 i_Color;
layout(location = 0) out vec4 o_Color;

void main() {
    o_Color = i_Color;
}