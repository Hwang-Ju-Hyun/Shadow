#version 400

in vec2 UV;

uniform sampler2D viewport_texture;

out vec4 FragColor;

void main()
{
	FragColor=texture(viewport_texture,UV);	
	return;
}