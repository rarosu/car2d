#version 440

in vec2 vs_texcoord;
out vec4 out_color;

layout(binding = 0) uniform sampler2D sampler_atlas;

layout(binding = 2, std140) uniform PerInstance
{
	mat3 model_matrix;
	vec4 color;
};

void main()
{
	vec4 font_color = texture(sampler_atlas, vs_texcoord);
	//out_color = vec4(font_color.r * color.rgb, font_color.a * color.a);
	//out_color = texture(sampler_atlas, vs_texcoord);
	out_color = vec4(color.rgb, color.a * font_color.r);
}