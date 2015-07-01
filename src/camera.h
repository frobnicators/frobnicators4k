#pragma once

#ifndef ENABLE_CAMERA
#error "camera.h included when ENABLE_CAMERA is not defined"
#endif

#include "frob_math.h"

typedef struct camera_t {
	float fovy;
	float aspect;
	float znear;
	float zfar;
	vec3 position;
	vec3 look_direction; // must be normalized

	mat4 view_matrix;
	mat4 proj_matrix;
	mat4 view_proj_matrix;
} camera_t;

void update_camera_matrices(camera_t* camera);