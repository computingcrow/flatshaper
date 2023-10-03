#version 330 core

uniform mat4 um_model;
uniform mat4 um_view;
uniform mat4 um_projection;

in vec3 iv_position;
in vec2 iv_uv;

out vec2 ov_uv;

void main() {
    ov_uv = iv_uv;
    gl_Position = um_projection * um_view * um_model * vec4(iv_position, 1.0);
}

