#version 400

layout(location = 0) in vec4 vPosition;
layout(location = 2) in vec2 vTextCoords;

out vec2 UV;

void main()
{
	gl_Position=vec4(vPosition.x*2.0,vPosition.y*2.0,0,1);
	UV=vTextCoords;
}