#include "ShadowMap.h"
#include <iostream>

ShadowMap::ShadowMap(int _width, int _height)
	:m_iShadowMapWidth(_width),
	m_iShadowMapHeight(_height),
	m_iShadowMapTextureUnit(0)
{	

	glGenFramebuffers(1, &m_iFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFBO);
	
	glGenTextures(1, &m_iShadowMapTextureID);
	glBindTexture(GL_TEXTURE_2D, m_iShadowMapTextureID);	
	glNamedFramebufferTexture(m_iFBO, GL_DEPTH_ATTACHMENT, m_iShadowMapTextureID, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_iShadowMapWidth, m_iShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer is not complete!"<<std::endl;
	}
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_iShadowMapTextureID, 0);

	//우리가 추가한 프레임버퍼는 화면에 그리기 위한 용도가 아님을 알림
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMap::~ShadowMap()
{
	glDeleteFramebuffers(1, &m_iFBO);
	glDeleteTextures(1, &m_iShadowMapTextureID);	
}

void ShadowMap::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFBO);
}

void ShadowMap::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::Read()
{
	
	glBindTextureUnit(m_iShadowMapTextureUnit, m_iShadowMapTextureID);
}
