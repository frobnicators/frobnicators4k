#ifndef FBO_H
#define FBO_H

#include "demo.h"

#ifndef ENABLE_FBOS
#error "Fbos must be enabled in demo.h before fbo.h is included"
#endif

#include "klister.h"

enum TextureType {
	TextureType_Color0=0,
	TextureType_Color1,
	TextureType_Depth
};

typedef struct {
	GLuint fbo;
	GLuint textures[3];
	unsigned int back_buffer;
} fbo_t;

// @param depth: 1 to enable depth. MUST NOT BE more than 1
void create_fbo(int w, int h, GLenum internalformat, GLenum format, GLenum type, unsigned int depth, fbo_t * fbo);

#endif