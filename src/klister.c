#include "klister.h"

#define getProcAddr wglGetProcAddress

PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLGETSHADERIVPROC glGetShaderiv = NULL;

#if _DEBUG
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
#endif

GLboolean initKlister() {
	GLboolean r = GL_FALSE;
	r = ( (glCreateShader = (PFNGLCREATESHADERPROC)getProcAddr("glCreateShader")) == NULL) || r;
	r = ( (glShaderSource = (PFNGLSHADERSOURCEPROC)getProcAddr("glShaderSource")) == NULL) || r;
	r = ( (glCompileShader = (PFNGLCOMPILESHADERPROC)getProcAddr("glCompileShader")) == NULL) || r;
	r = ( (glGetShaderiv = (PFNGLGETSHADERIVPROC)getProcAddr("glGetShaderiv")) == NULL) || r;

#if _DEBUG
	r = ( (glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)getProcAddr("glGetShaderInfoLog")) == NULL) || r;
#endif
	return !r;
}