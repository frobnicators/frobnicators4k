#ifndef KLISTER_H
#define KLISTER_H

#include "winclude.h"
#include <GL/gl.h>

typedef unsigned char GLboolean;
typedef char GLchar;

/* Manual glew stuff */

void initKlister();

/* Defines */
#define GL_BLEND_EQUATION_RGB GL_BLEND_EQUATION
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED 0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE 0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE 0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE 0x8625
#define GL_CURRENT_VERTEX_ATTRIB 0x8626
#define GL_VERTEX_PROGRAM_POINT_SIZE 0x8642
#define GL_VERTEX_PROGRAM_TWO_SIDE 0x8643
#define GL_VERTEX_ATTRIB_ARRAY_POINTER 0x8645
#define GL_STENCIL_BACK_FUNC 0x8800
#define GL_STENCIL_BACK_FAIL 0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL 0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS 0x8803
#define GL_MAX_DRAW_BUFFERS 0x8824
#define GL_DRAW_BUFFER0 0x8825
#define GL_DRAW_BUFFER1 0x8826
#define GL_DRAW_BUFFER2 0x8827
#define GL_DRAW_BUFFER3 0x8828
#define GL_DRAW_BUFFER4 0x8829
#define GL_DRAW_BUFFER5 0x882A
#define GL_DRAW_BUFFER6 0x882B
#define GL_DRAW_BUFFER7 0x882C
#define GL_DRAW_BUFFER8 0x882D
#define GL_DRAW_BUFFER9 0x882E
#define GL_DRAW_BUFFER10 0x882F
#define GL_DRAW_BUFFER11 0x8830
#define GL_DRAW_BUFFER12 0x8831
#define GL_DRAW_BUFFER13 0x8832
#define GL_DRAW_BUFFER14 0x8833
#define GL_DRAW_BUFFER15 0x8834
#define GL_BLEND_EQUATION_ALPHA 0x883D
#define GL_POINT_SPRITE 0x8861
#define GL_COORD_REPLACE 0x8862
#define GL_MAX_VERTEX_ATTRIBS 0x8869
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED 0x886A
#define GL_MAX_TEXTURE_COORDS 0x8871
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS 0x8B49
#define GL_MAX_VERTEX_UNIFORM_COMPONENTS 0x8B4A
#define GL_MAX_VARYING_FLOATS 0x8B4B
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS 0x8B4C
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_SHADER_TYPE 0x8B4F
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_BOOL 0x8B56
#define GL_BOOL_VEC2 0x8B57
#define GL_BOOL_VEC3 0x8B58
#define GL_BOOL_VEC4 0x8B59
#define GL_FLOAT_MAT2 0x8B5A
#define GL_FLOAT_MAT3 0x8B5B
#define GL_FLOAT_MAT4 0x8B5C
#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_2D 0x8B5E
#define GL_SAMPLER_3D 0x8B5F
#define GL_SAMPLER_CUBE 0x8B60
#define GL_SAMPLER_1D_SHADOW 0x8B61
#define GL_SAMPLER_2D_SHADOW 0x8B62
#define GL_DELETE_STATUS 0x8B80
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_ATTACHED_SHADERS 0x8B85
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH 0x8B87
#define GL_SHADER_SOURCE_LENGTH 0x8B88
#define GL_ACTIVE_ATTRIBUTES 0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH 0x8B8A
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#define GL_CURRENT_PROGRAM 0x8B8D
#define GL_POINT_SPRITE_COORD_ORIGIN 0x8CA0
#define GL_LOWER_LEFT 0x8CA1
#define GL_UPPER_LEFT 0x8CA2
#define GL_STENCIL_BACK_REF 0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK 0x8CA4
#define GL_STENCIL_BACK_WRITEMASK 0x8CA5

/* typedefs */
typedef void ( * PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void ( * PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar* name);
typedef void ( * PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum, GLenum);
typedef void ( * PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint ( * PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint ( * PFNGLCREATESHADERPROC) (GLenum type);
typedef void ( * PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void ( * PFNGLDELETESHADERPROC) (GLuint shader);
typedef void ( * PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void ( * PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint);
typedef void ( * PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum* bufs);
typedef void ( * PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint);
typedef void ( * PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void ( * PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void ( * PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei* count, GLuint* shaders);
typedef GLint ( * PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void ( * PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void ( * PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint* param);
typedef void ( * PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void ( * PFNGLGETSHADERSOURCEPROC) (GLuint obj, GLsizei maxLength, GLsizei* length, GLchar* source);
typedef void ( * PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint* param);
typedef GLint ( * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void ( * PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat* params);
typedef void ( * PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint* params);
typedef void ( * PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint, GLenum, GLvoid**);
typedef void ( * PFNGLGETVERTEXATTRIBDVPROC) (GLuint, GLenum, GLdouble*);
typedef void ( * PFNGLGETVERTEXATTRIBFVPROC) (GLuint, GLenum, GLfloat*);
typedef void ( * PFNGLGETVERTEXATTRIBIVPROC) (GLuint, GLenum, GLint*);
typedef GLboolean ( * PFNGLISPROGRAMPROC) (GLuint program);
typedef GLboolean ( * PFNGLISSHADERPROC) (GLuint shader);
typedef void ( * PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void ( * PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths);
typedef void ( * PFNGLSTENCILFUNCSEPARATEPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
typedef void ( * PFNGLSTENCILMASKSEPARATEPROC) (GLenum, GLuint);
typedef void ( * PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void ( * PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void ( * PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void ( * PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void ( * PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void ( * PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
typedef void ( * PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void ( * PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
typedef void ( * PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void ( * PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void ( * PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void ( * PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
typedef void ( * PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void ( * PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void ( * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void ( * PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void ( * PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void ( * PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void ( * PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void ( * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
typedef void ( * PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void ( * PFNGLVALIDATEPROGRAMPROC) (GLuint program);
typedef void ( * PFNGLVERTEXATTRIB1DPROC) (GLuint index, GLdouble x);
typedef void ( * PFNGLVERTEXATTRIB1DVPROC) (GLuint index, const GLdouble* v);
typedef void ( * PFNGLVERTEXATTRIB1FPROC) (GLuint index, GLfloat x);
typedef void ( * PFNGLVERTEXATTRIB1FVPROC) (GLuint index, const GLfloat* v);
typedef void ( * PFNGLVERTEXATTRIB1SPROC) (GLuint index, GLshort x);
typedef void ( * PFNGLVERTEXATTRIB1SVPROC) (GLuint index, const GLshort* v);
typedef void ( * PFNGLVERTEXATTRIB2DPROC) (GLuint index, GLdouble x, GLdouble y);
typedef void ( * PFNGLVERTEXATTRIB2DVPROC) (GLuint index, const GLdouble* v);
typedef void ( * PFNGLVERTEXATTRIB2FPROC) (GLuint index, GLfloat x, GLfloat y);
typedef void ( * PFNGLVERTEXATTRIB2FVPROC) (GLuint index, const GLfloat* v);
typedef void ( * PFNGLVERTEXATTRIB2SPROC) (GLuint index, GLshort x, GLshort y);
typedef void ( * PFNGLVERTEXATTRIB2SVPROC) (GLuint index, const GLshort* v);
typedef void ( * PFNGLVERTEXATTRIB3DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void ( * PFNGLVERTEXATTRIB3DVPROC) (GLuint index, const GLdouble* v);
typedef void ( * PFNGLVERTEXATTRIB3FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void ( * PFNGLVERTEXATTRIB3FVPROC) (GLuint index, const GLfloat* v);
typedef void ( * PFNGLVERTEXATTRIB3SPROC) (GLuint index, GLshort x, GLshort y, GLshort z);
typedef void ( * PFNGLVERTEXATTRIB3SVPROC) (GLuint index, const GLshort* v);
typedef void ( * PFNGLVERTEXATTRIB4NBVPROC) (GLuint index, const GLbyte* v);
typedef void ( * PFNGLVERTEXATTRIB4NIVPROC) (GLuint index, const GLint* v);
typedef void ( * PFNGLVERTEXATTRIB4NSVPROC) (GLuint index, const GLshort* v);
typedef void ( * PFNGLVERTEXATTRIB4NUBPROC) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void ( * PFNGLVERTEXATTRIB4NUBVPROC) (GLuint index, const GLubyte* v);
typedef void ( * PFNGLVERTEXATTRIB4NUIVPROC) (GLuint index, const GLuint* v);
typedef void ( * PFNGLVERTEXATTRIB4NUSVPROC) (GLuint index, const GLushort* v);
typedef void ( * PFNGLVERTEXATTRIB4BVPROC) (GLuint index, const GLbyte* v);
typedef void ( * PFNGLVERTEXATTRIB4DPROC) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void ( * PFNGLVERTEXATTRIB4DVPROC) (GLuint index, const GLdouble* v);
typedef void ( * PFNGLVERTEXATTRIB4FPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void ( * PFNGLVERTEXATTRIB4FVPROC) (GLuint index, const GLfloat* v);
typedef void ( * PFNGLVERTEXATTRIB4IVPROC) (GLuint index, const GLint* v);
typedef void ( * PFNGLVERTEXATTRIB4SPROC) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void ( * PFNGLVERTEXATTRIB4SVPROC) (GLuint index, const GLshort* v);
typedef void ( * PFNGLVERTEXATTRIB4UBVPROC) (GLuint index, const GLubyte* v);
typedef void ( * PFNGLVERTEXATTRIB4UIVPROC) (GLuint index, const GLuint* v);
typedef void ( * PFNGLVERTEXATTRIB4USVPROC) (GLuint index, const GLushort* v);
typedef void ( * PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);

/* 1.5 */
#define GL_FOG_COORD_SRC GL_FOG_COORDINATE_SOURCE
#define GL_FOG_COORD GL_FOG_COORDINATE
#define GL_FOG_COORD_ARRAY GL_FOG_COORDINATE_ARRAY
#define GL_SRC0_RGB GL_SOURCE0_RGB
#define GL_FOG_COORD_ARRAY_POINTER GL_FOG_COORDINATE_ARRAY_POINTER
#define GL_FOG_COORD_ARRAY_TYPE GL_FOG_COORDINATE_ARRAY_TYPE
#define GL_SRC1_ALPHA GL_SOURCE1_ALPHA
#define GL_CURRENT_FOG_COORD GL_CURRENT_FOG_COORDINATE
#define GL_FOG_COORD_ARRAY_STRIDE GL_FOG_COORDINATE_ARRAY_STRIDE
#define GL_SRC0_ALPHA GL_SOURCE0_ALPHA
#define GL_SRC1_RGB GL_SOURCE1_RGB
#define GL_FOG_COORD_ARRAY_BUFFER_BINDING GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING
#define GL_SRC2_ALPHA GL_SOURCE2_ALPHA
#define GL_SRC2_RGB GL_SOURCE2_RGB
#define GL_BUFFER_SIZE 0x8764
#define GL_BUFFER_USAGE 0x8765
#define GL_QUERY_COUNTER_BITS 0x8864
#define GL_CURRENT_QUERY 0x8865
#define GL_QUERY_RESULT 0x8866
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_ARRAY_BUFFER_BINDING 0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING 0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING 0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING 0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING 0x8898
#define GL_INDEX_ARRAY_BUFFER_BINDING 0x8899
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A
#define GL_EDGE_FLAG_ARRAY_BUFFER_BINDING 0x889B
#define GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING 0x889C
#define GL_FOG_COORDINATE_ARRAY_BUFFER_BINDING 0x889D
#define GL_WEIGHT_ARRAY_BUFFER_BINDING 0x889E
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_BUFFER_ACCESS 0x88BB
#define GL_BUFFER_MAPPED 0x88BC
#define GL_BUFFER_MAP_POINTER 0x88BD
#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA
#define GL_SAMPLES_PASSED 0x8914

typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

typedef void ( * PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
typedef void ( * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void ( * PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef void ( * PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
typedef void ( * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
typedef void ( * PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint* ids);
typedef void ( * PFNGLENDQUERYPROC) (GLenum target);
typedef void ( * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void ( * PFNGLGENQUERIESPROC) (GLsizei n, GLuint* ids);
typedef void ( * PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef void ( * PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname, GLvoid** params);
typedef void ( * PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data);
typedef void ( * PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint* params);
typedef void ( * PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint* params);
typedef void ( * PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef GLboolean ( * PFNGLISBUFFERPROC) (GLuint buffer);
typedef GLboolean ( * PFNGLISQUERYPROC) (GLuint id);
typedef GLvoid* ( * PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
typedef GLboolean ( * PFNGLUNMAPBUFFERPROC) (GLenum target);

/* Functions */
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

#if SOME_DEBUG
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
#endif

#endif