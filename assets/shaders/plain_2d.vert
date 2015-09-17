#version 140

in vec2 in_position_m;

layout(std140) uniform PerFrame
{
	mat3 view_matrix;
	mat3 projection_matrix;
};

layout(std140) uniform PerInstance
{
	mat3 model_matrix;
};

void main()
{
	gl_Position = vec4((projection_matrix * view_matrix * model_matrix * vec3(in_position_m, 1.0f)).xyz, 1.0f);
}