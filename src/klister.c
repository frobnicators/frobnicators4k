#include "klister.h"
#include "debug.h"
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
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FVPROC glUniform3fv;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM2IPROC glUniform2i;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLMEMORYBARRIERPROC glMemoryBarrier;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
PFNGLUNMAPBUFFERPROC glUnmapBuffer;

#ifdef ENABLE_FBOS
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
#endif

#ifdef ENABLE_TEXTURES
PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif

#ifdef ENABLE_COMPUTE
PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
#endif

#if SOME_DEBUG
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
#endif

void initKlister() {
	FROB_PRINTF("initKlister()\n");
	glCreateShader = (PFNGLCREATESHADERPROC)getProcAddr("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)getProcAddr("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)getProcAddr("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)getProcAddr("glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)getProcAddr("glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)getProcAddr("glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)getProcAddr("glUseProgram");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)getProcAddr("glGetUniformLocation");
	glUniform1f = (PFNGLUNIFORM1FPROC)getProcAddr("glUniform1f");
	glUniform2f = (PFNGLUNIFORM2FPROC)getProcAddr("glUniform2f");
	glUniform3fv = (PFNGLUNIFORM3FVPROC)getProcAddr("glUniform3fv");
	glUniform1i = (PFNGLUNIFORM1IPROC)getProcAddr("glUniform1i");
	glUniform2i = (PFNGLUNIFORM2IPROC)getProcAddr("glUniform2i");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)getProcAddr("glUniformMatrix4fv");
	glGenBuffers = (PFNGLGENBUFFERSPROC)getProcAddr("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)getProcAddr("glBindBuffer");
	glBufferSubData = (PFNGLBUFFERSUBDATAPROC)getProcAddr("glBufferSubData");
	glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)getProcAddr("glBindBufferBase");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)getProcAddr("glEnableVertexAttribArray");
	glBufferData = (PFNGLBUFFERDATAPROC)getProcAddr("glBufferData");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)getProcAddr("glVertexAttribPointer");
	glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)getProcAddr("glVertexAttribIPointer");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)getProcAddr("glBindVertexArray");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)getProcAddr("glDeleteVertexArrays");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)getProcAddr("glGenVertexArrays");
	glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)getProcAddr("glMemoryBarrier");
	glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)getProcAddr("glMapBufferRange");
	glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)getProcAddr("glUnmapBuffer");
#ifdef ENABLE_FBOS
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)getProcAddr("glGenFramebuffers");
	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)getProcAddr("glBindFramebuffer");
	glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)getProcAddr("glFramebufferTexture2D");
#endif

#ifdef ENABLE_TEXTURES
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)getProcAddr("glActiveTexture");
#endif

#ifdef ENABLE_COMPUTE
	glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)getProcAddr("glDispatchCompute");
#endif


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
		glUniform2f == NULL ||
		glUniform3fv == NULL ||
		glUniform1i == NULL ||
		glUniform2i == NULL ||
		glUniformMatrix4fv == NULL ||
		glGenBuffers == NULL ||
		glBindBuffer == NULL ||
		glBindBufferBase == NULL ||
		glEnableVertexAttribArray == NULL ||
		glBufferData == NULL ||
		glBufferSubData == NULL ||
		glVertexAttribPointer == NULL ||
		glVertexAttribIPointer == NULL ||
		glGetShaderiv == NULL ||
		glGetShaderInfoLog == NULL ||
		glGetProgramiv == NULL ||
		glGetProgramInfoLog == NULL ||
		glGenVertexArrays == NULL ||
		glBindVertexArray == NULL ||
		glDeleteVertexArrays == NULL ||
		glMemoryBarrier == NULL
#ifdef ENABLE_FBOS
		|| glGenFramebuffers == NULL
		|| glBindFramebuffer == NULL
		|| glFramebufferTexture2D == NULL
#endif

#ifdef ENABLE_COMPUTE
		|| glDispatchCompute == NULL
#endif

		) {
			FROB_ERROR("GL Init Error", "Failed to get GL functions");
#if _DEBUG
			terminate();
#endif
	}
#endif
}