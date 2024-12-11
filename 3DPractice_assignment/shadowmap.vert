#version 400

layout(location=0) in vec4 vPosition;

uniform mat4 model;
uniform mat4 modeltoworld;

void main()
{
	gl_Position= model * vPosition;
}