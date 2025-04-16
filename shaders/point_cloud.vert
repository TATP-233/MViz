#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 fragColor;

uniform mat4 model;
uniform mat4 view_projection;
uniform float point_size;

void main() {
    gl_Position = view_projection * model * vec4(aPos, 1.0);
    gl_PointSize = point_size;
    fragColor = aColor;
} 