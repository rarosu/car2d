#version 140

in vec2 in_position_m;
in vec2 in_texcoord;

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
	mat3 inverse_projection_view;
};

void main()
{
	gl_Position = vec4(in_position_m, 0.0f, 1.0f);
	vs_texcoord = (model_matrix * inverse_projection_view * bias_matrix * vec3(in_texcoord, 1.0f)).xy;
}