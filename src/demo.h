#ifndef DEMO_H
#define DEMO_H
#define DEMO_FOLDER "frist"
	void init_demo();
	void render_demo();

//#define ENABLE_TEXTURES 1
//#define ENABLE_FBOS 1

#define STATIC_WIDTH 1280
#define STATIC_HEIGHT 720

#define INIT_GL_NEEDED defined(ENABLE_TEXTURES) /* Todo, any other thing that we need init_gl for */

#define DEMO_LENGTH 84.5f

#endif