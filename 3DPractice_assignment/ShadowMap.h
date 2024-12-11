#pragma once
#include "GL/glew.h"

class ShadowMap
{
public:
	ShadowMap(int _width, int _height);
	~ShadowMap();
public:
	unsigned int m_iFBO;
	unsigned int m_iShadowMapTextureID;
	unsigned int m_iShadowMapWidth;
	unsigned int m_iShadowMapHeight;
	GLuint m_iShadowMapTextureUnit;
public:
	void Bind();
	void UnBind();
	void Read();
};

