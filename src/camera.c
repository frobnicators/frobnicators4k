#include "demo.h"

#ifdef ENABLE_CAMERA
#include "camera.h"
#include <string.h>

void update_camera_matrices(camera_t* camera) {
	// view matrix:
	vec3 localy = { 0.f, 1.f, 0.f };
	vec3 localx = cross(&camera->look_direction, &localy);
	normal_v3(&localx);
	localy = cross(&localx, &camera->look_direction);
	normal_v3(&localy);

	mat4 rot;
	memset(&rot, 0, sizeof(mat4));
	memcpy(&rot.c0, &localx, sizeof(float) * 3);
	memcpy(&rot.c1, &localy, sizeof(float) * 3);
	memcpy(&rot.c2, &camera->look_direction, sizeof(float) * 3);
	rot.c3.w = 1;

	mat4 tr;
	memset(&tr, 0, sizeof(mat4));
	tr.c0.x = 1;
	tr.c1.y = 1;
	tr.c2.z = 1;
	tr.c3.x = -camera->position.x;
	tr.c3.y = -camera->position.y;
	tr.c3.z = -camera->position.z;
	tr.c3.w = 1.f;

	camera->view_matrix = mulmm(&tr, &rot);
	//camera->view_matrix = mulmm(&rot, &tr);

	/*
	memset(&camera->view_matrix, 0, sizeof(mat4));
	memcpy(&camera->view_matrix.c0, &localx, sizeof(float)*3);
	memcpy(&camera->view_matrix.c1, &localy, sizeof(float)*3);
	memcpy(&camera->view_matrix.c2, &camera->look_direction, sizeof(float)*3);
	camera->view_matrix.c3.x = -dotv3(&localx, &camera->position);
	camera->view_matrix.c3.y = -dotv3(&localy, &camera->position);
	camera->view_matrix.c3.z = -dotv3(&camera->look_direction, &camera->position);
	camera->view_matrix.c3.w = 1.f;
	*/

	// projection matrix
	float f = (float)(1.f / tan(camera->fovy));
	float d = (camera->znear - camera->zfar);

	memset(&camera->proj_matrix, 0, sizeof(mat4));
	camera->proj_matrix.c0.x = f/camera->aspect;
	camera->proj_matrix.c1.y = f;
	camera->proj_matrix.c2.z = (camera->zfar + camera->znear) / d;
	camera->proj_matrix.c2.w = -1.f;
	camera->proj_matrix.c3.z = 2.f * camera->zfar * camera->znear / d;

	camera->view_proj_matrix = mulmm(&camera->view_matrix, &camera->proj_matrix);
}

#endif