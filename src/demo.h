#ifndef DEMO_H
#define DEMO_H
#define DEMO_FOLDER "edison2015"
void init_demo();
void render_demo();
void upload_light(struct shader_t* shader);

#define ENABLE_MATH

#define ENABLE_TEXTURES 1
#define ENABLE_FBOS 1
#define ENABLE_CAMERA 1
#define ENABLE_INSTANCED_DRAWING 1

#define ENABLE_COMPUTE

#define OCEAN_DEBUG 1

// Defining static width saves some bytes
//#define STATIC_WIDTH 1920
//#define STATIC_HEIGHT 1080

#define INIT_GL_NEEDED defined(ENABLE_TEXTURES) /* Todo, any other thing that we need init_gl for */

#define DEMO_LENGTH 284.5f

extern struct camera_t camera;
extern struct fbo_t main_fbo;


#endif