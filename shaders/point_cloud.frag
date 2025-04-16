#version 330 core

in vec3 fragColor;
out vec4 FragColor;

void main() {
    // 为点创建平滑的圆形
    vec2 coord = gl_PointCoord - vec2(0.5);
    if(length(coord) > 0.5)
        discard;
        
    FragColor = vec4(fragColor, 1.0);
} 