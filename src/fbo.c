#include "demo.h"	
#if ENABLE_FBOS

#include "fbo.h"
#include "shader.h"

#include <stdlib.h>

#ifndef ENABLE_TEXTURES
#error "Textures must be enabled for fbos to work"
#endif

void create_fbo(int w, int h, GLenum format, struct fbo_t * fbo) {
	glGenFramebuffers(1, &(fbo->fbo));
	glGenTextures(1, &(fbo->texture));
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
	init_gl();
	glBindTexture(GL_TEXTURE_2D, fbo->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format == GL_RGB8 ? GL_RGB : GL_RGBA, GL_UNSIGNED_INT, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#endif