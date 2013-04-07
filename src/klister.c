#include "klister.h"
#if SOME_DEBUG
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
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

#if SOME_DEBUG
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
	glGenBuffers = (PFNGLGENBUFFERSPROC)getProcAddr("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)getProcAddr("glBindBuffer");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)getProcAddr("glEnableVertexAttribArray");
	glBufferData = (PFNGLBUFFERDATAPROC)getProcAddr("glBufferData");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)getProcAddr("glVertexAttribPointer");

#if SOME_DEBUG
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
		glGenBuffers == NULL ||
		glBindBuffer == NULL ||
		glEnableVertexAttribArray == NULL ||
		glBufferData == NULL ||
		glVertexAttribPointer == NULL ||

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