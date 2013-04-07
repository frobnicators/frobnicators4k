#include "klister.h"
#if _DEBUG
#include "main.h"
#endif

#define getProcAddr wglGetProcAddress

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

#if _DEBUG
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
#endif

void initKlister() {
	glCreateShader = (PFNGLCREATESHADERPROC)getProcAddr("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)getProcAddr("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)getProcAddr("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)getProcAddr("glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)getProcAddr("glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)getProcAddr("glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)getProcAddr("glUseProgram");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)getProcAddr("glGetUniformLocation");
	glUniform1f = (PFNGLUNIFORM1FPROC)getProcAddr("glUniform1f");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)getProcAddr("glUniformMatrix4fv");

#if _DEBUG
	glGetShaderiv = (PFNGLGETSHADERIVPROC)getProcAddr("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)getProcAddr("glGetShaderInfoLog");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)getProcAddr("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)getProcAddr("glGetProgramInfoLog");

	if(
		glCreateShader == NULL ||
		glShaderSource == NULL ||
		glCompileShader == NULL ||
		glCreateProgram == NULL ||
		glAttachShader == NULL ||
		glLinkProgram == NULL ||
		glUseProgram == NULL ||
		glGetUniformLocation == NULL ||
		glUniform1f == NULL ||
		glUniformMatrix4fv == NULL ||

		glGetShaderiv == NULL ||
		glGetShaderInfoLog == NULL ||
		glGetProgramiv == NULL ||
		glGetProgramInfoLog == NULL
		) {
			MessageBox(NULL, "Failed to get GL functions", "ERROR", MB_OK | MB_ICONEXCLAMATION);
			terminate();
	}
#endif
}