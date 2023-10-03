#version 330 core

uniform sampler2D ut_diffuse;
uniform sampler2D ut_normal;

in vec2 ov_uv;

out vec4 of_color;

void main() {
    of_color = texture(ut_diffuse, ov_uv) + (texture(ut_normal, (ov_uv * vec2(1, -1))) * 0.01);
}

