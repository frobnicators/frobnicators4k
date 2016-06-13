#include "demo.h"
#if ENABLE_FBOS

#include "fbo.h"
#include "shader.h"

#include <stdlib.h>

#ifndef ENABLE_TEXTURES
#error "Textures must be enabled for fbos to work"
#endif

void create_fbo(int w, int h, GLenum internalformat, GLenum format, GLenum type, unsigned int depth, fbo_t * fbo) {
	glGenFramebuffers(1, &(fbo->fbo));
	glGenTextures(1 + depth, fbo->textures);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
	init_gl();

	glBindTexture(GL_TEXTURE_2D, fbo->textures[TextureType_Color]);
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, w, h, 0, format, type, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glBindTexture(GL_TEXTURE_2D, fbo->textures[TextureType_Depth]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->textures[TextureType_Color], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fbo->textures[TextureType_Depth], 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#endif