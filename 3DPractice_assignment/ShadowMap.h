#pragma once

class ShadowMap
{
public:
	ShadowMap(int _width, int _height);
	~ShadowMap();
public:
	GLuint m_iShadowMapWidth;
	GLuint m_iShadowMapHeight;
	GLuint m_iFBO;
	GLuint m_iShadowMapTextureUnit;
	GLuint m_iShadowMapTextureID;
public:
	void Bind();
	void Read();
	void UnBind();
};