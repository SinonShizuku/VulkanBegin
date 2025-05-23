#version 460
#pragma shader_stage(vertex)

layout(location = 0) in vec2 i_Position;
layout(location = 1) in vec4 i_Color;
layout(location = 2) in vec2 i_InstancePosition;
layout(location = 0) out vec4 o_Color;

void main() {
    gl_Position = vec4(i_Position + i_InstancePosition, 0, 1);//共通顶点数据中的位置坐标 + 各个实例的位置偏移
    o_Color = i_Color;
}