#include "klister.h"

#define getProcAddr wglGetProcAddress

PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;

#if _DEBUG
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
#endif

GLboolean initKlister() {
	GLboolean r = GL_FALSE;
	r = ( (glCreateShader = (PFNGLCREATESHADERPROC)getProcAddr("glCreateShader")) == NULL) || r;
	r = ( (glShaderSource = (PFNGLSHADERSOURCEPROC)getProcAddr("glShaderSource")) == NULL) || r;
	r = ( (glCompileShader = (PFNGLCOMPILESHADERPROC)getProcAddr("glCompileShader")) == NULL) || r;
	r = ( (glCreateProgram = (PFNGLCREATEPROGRAMPROC)getProcAddr("glCreateProgram")) == NULL) || r;
	r = ( (glAttachShader = (PFNGLATTACHSHADERPROC)getProcAddr("glAttachShader")) == NULL) || r;
	r = ( (glLinkProgram = (PFNGLLINKPROGRAMPROC)getProcAddr("glLinkProgram")) == NULL) || r;

#if _DEBUG
	r = ( (glGetShaderiv = (PFNGLGETSHADERIVPROC)getProcAddr("glGetShaderiv")) == NULL) || r;
	r = ( (glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)getProcAddr("glGetShaderInfoLog")) == NULL) || r;
	r = ( (glGetProgramiv = (PFNGLGETPROGRAMIVPROC)getProcAddr("glGetProgramiv")) == NULL) || r;
	r = ( (glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)getProcAddr("glGetProgramInfoLog")) == NULL) || r;
#endif

	return !r;
}