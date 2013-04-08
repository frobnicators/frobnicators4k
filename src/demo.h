#ifndef DEMO_H
#define DEMO_H
#define DEMO_FOLDER "demo"
	void init_demo();
	void update_demo();
	void render_demo();

#define ENABLE_TEXTURES 1
#define ENABLE_FBOS 1

#define INIT_GL_NEEDED defined(ENABLE_TEXTURES) /* Todo, any other thing that we need init_gl for */

#endif