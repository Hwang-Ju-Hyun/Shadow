#pragma once
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
public:
	void Bind();
	void UnBind();
};

