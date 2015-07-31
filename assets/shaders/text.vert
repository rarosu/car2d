#version 440

layout(location = 0) in vec2 in_position_s;
layout(location = 1) in vec2 in_texcoord;

out vec2 vs_texcoord;

layout(binding = 2, std140) uniform PerInstance
{
	mat3 model_matrix;
	vec4 color;
};

void main()
{
	gl_Position = vec4((model_matrix * vec3(in_position_s, 1.0f)).xy, 0.0f, 1.0f);
	vs_texcoord = in_texcoord;
}