#version 440

in vec2 vs_texcoord;
out vec4 out_color;

//layout(binding = 0) uniform sampler2D sampler_color;

void main()
{
	//out_color = texture(sampler_color, vs_texcoord);
	out_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}