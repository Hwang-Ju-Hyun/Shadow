#include "GL/glew.h"
#include "ShadowMap.h"
#include <iostream>

ShadowMap::ShadowMap(int _width, int _height)
	:m_iShadowMapWidth(_width),
	m_iShadowMapHeight(_height)	
{	
	glGenFramebuffers(1, &m_iFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_iFBO);
	
	glGenTextures(1, &m_iShadowMapTextureID);
	glBindTexture(GL_TEXTURE_2D, m_iShadowMapTextureID);	

	glNamedFramebufferTexture(m_iFBO, GL_DEPTH_ATTACHMENT, m_iShadowMapTextureID, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_iShadowMapWidth, m_iShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer is not complete!"<<std::endl;
	}

	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_iShadowMapTextureID, 0);

	//�츮�� �߰��� �����ӹ��۴� ȭ�鿡 �׸��� ���� �뵵�� �ƴ��� �˸�
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMap::~ShadowMap()
{
	glDeleteFramebuffers(1,&m_iFBO);
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
