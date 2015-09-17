#version 330

layout(location = 0) in vec2 in_position_m;
layout(location = 1) in vec2 in_texcoord;

out vec2 vs_texcoord;

layout(std140) uniform PerFrame
{
	mat3 view_matrix;
	mat3 projection_matrix;
};

layout(std140) uniform PerInstance
{
	mat3 model_matrix;
	mat3 bias_matrix;
};

void main()
{
	gl_Position = vec4(in_position_m, 0.0f, 1.0f);
	vs_texcoord = (model_matrix * inverse(projection_matrix * view_matrix) * bias_matrix * vec3(in_texcoord, 1.0f)).xy;
}