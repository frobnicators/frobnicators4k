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
//PFNGLUNIFORM1IPROC glUniform1i;
//PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
//PFNGLACTIVETEXTUREPROC glActiveTexture;

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
	//glUniform1i = (PFNGLUNIFORM1IPROC)getProcAddr("glUniform1i");
	//glUniform2i = (PFNGLUNIFORM2IPROC)getProcAddr("glUniform2i");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)getProcAddr("glUniformMatrix4fv");
	glGenBuffers = (PFNGLGENBUFFERSPROC)getProcAddr("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)getProcAddr("glBindBuffer");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)getProcAddr("glEnableVertexAttribArray");
	glBufferData = (PFNGLBUFFERDATAPROC)getProcAddr("glBufferData");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)getProcAddr("glVertexAttribPointer");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)getProcAddr("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)getProcAddr("glBindFramebuffer");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)getProcAddr("glFramebufferTexture2D");
	//glActiveTexture = (PFNGLACTIVETEXTUREPROC)getProcAddr("glActiveTexture");

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
		//glUniform1i == NULL ||
		//glUniform2i == NULL ||
		glUniformMatrix4fv == NULL ||
		glGenBuffers == NULL ||
		glBindBuffer == NULL ||
		glEnableVertexAttribArray == NULL ||
		glBufferData == NULL ||
		glVertexAttribPointer == NULL ||
		glGenFramebuffers == NULL ||
		glBindFramebuffer == NULL ||
		glFramebufferTexture2D == NULL ||
		//glActiveTexture == NULL ||

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