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
	//normal_v3(&localy);

	memset(&camera->view_matrix, 0, sizeof(mat4));

	camera->view_matrix.c0.x = localx.x;
	camera->view_matrix.c1.x = localx.y;
	camera->view_matrix.c2.x = localx.z;

	camera->view_matrix.c0.y = localy.x;
	camera->view_matrix.c1.y = localy.y;
	camera->view_matrix.c2.y = localy.z;

	camera->view_matrix.c0.z = camera->look_direction.x;
	camera->view_matrix.c1.z = camera->look_direction.y;
	camera->view_matrix.c2.z = camera->look_direction.z;

	camera->view_matrix.c3.x = -dotv3(&localx, &camera->position);
	camera->view_matrix.c3.y = -dotv3(&localy, &camera->position);
	camera->view_matrix.c3.z = -dotv3(&camera->look_direction, &camera->position);

	camera->view_matrix.c3.w = 1.f;

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