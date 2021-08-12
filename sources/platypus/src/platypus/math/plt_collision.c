#include "plt_collision.h"

#include <math.h>

// Seperate Axis Theorem Box Collision
// source: https://github.com/idmillington/cyclone-physics
// MIT License

Plt_Vector3f plt_box_get_half_size(Plt_Shape_Box box) {
	return (Plt_Vector3f) {
		box.size.x / 2.0f,
		box.size.y / 2.0f,
		box.size.z / 2.0f
	};
}

Plt_Vector3f plt_matrix_get_axis(Plt_Matrix4x4f m, unsigned int axis) {
	return (Plt_Vector3f) {
		.x = m.columns[axis][0],
		.y = m.columns[axis][1],
		.z = m.columns[axis][2]
	};
}

float plt_box_transform_to_axis(Plt_Shape_Box box, Plt_Matrix4x4f transform, Plt_Vector3f axis) {
	Plt_Vector3f half_size = plt_box_get_half_size(box);
	return half_size.x * fabsf(plt_vector3f_dot_product(axis, plt_matrix_get_axis(transform, 0))) +
		half_size.y * fabsf(plt_vector3f_dot_product(axis, plt_matrix_get_axis(transform, 1))) +
		half_size.z * fabsf(plt_vector3f_dot_product(axis, plt_matrix_get_axis(transform, 2)));
}

bool plt_box_overlap_on_axis(Plt_Matrix4x4f transform_a, Plt_Matrix4x4f transform_b, Plt_Shape_Box box_a, Plt_Shape_Box box_b, Plt_Vector3f axis, Plt_Vector3f to_centre)
{
    // Project the half-size of one onto axis
    float project_a = plt_box_transform_to_axis(box_a, transform_a, axis);
    float project_b = plt_box_transform_to_axis(box_b, transform_b, axis);

    // Project this onto the axis
    float distance = fabsf(plt_vector3f_dot_product(to_centre, axis));

    // Check for overlap
    return distance < (project_a + project_b);
}

#define TEST_OVERLAP(axis) plt_box_overlap_on_axis(transform_a, transform_b, box_a, box_b, axis, to_centre)

bool plt_collision_box_box(Plt_Matrix4x4f transform_a, Plt_Matrix4x4f transform_b, Plt_Shape_Box box_a, Plt_Shape_Box box_b) {
	// Find the vector between the two centres
	Plt_Vector3f to_centre = plt_vector3f_subtract(plt_matrix_get_axis(transform_b, 3), plt_matrix_get_axis(transform_a, 3));
	
	return (
		// Check on box one's axes first
		TEST_OVERLAP(plt_matrix_get_axis(transform_a, 0)) &&
		TEST_OVERLAP(plt_matrix_get_axis(transform_a, 1)) &&
		TEST_OVERLAP(plt_matrix_get_axis(transform_a, 2)) &&

		// And on two's
		TEST_OVERLAP(plt_matrix_get_axis(transform_b, 0)) &&
		TEST_OVERLAP(plt_matrix_get_axis(transform_b, 1)) &&
		TEST_OVERLAP(plt_matrix_get_axis(transform_b, 2)) &&

		// Now on the cross products
		TEST_OVERLAP(plt_vector3f_cross(plt_matrix_get_axis(transform_a, 0), plt_matrix_get_axis(transform_b, 0))) &&
		TEST_OVERLAP(plt_vector3f_cross(plt_matrix_get_axis(transform_a, 0), plt_matrix_get_axis(transform_b, 1))) &&
		TEST_OVERLAP(plt_vector3f_cross(plt_matrix_get_axis(transform_a, 0), plt_matrix_get_axis(transform_b, 2))) &&
		TEST_OVERLAP(plt_vector3f_cross(plt_matrix_get_axis(transform_a, 1), plt_matrix_get_axis(transform_b, 0))) &&
		TEST_OVERLAP(plt_vector3f_cross(plt_matrix_get_axis(transform_a, 1), plt_matrix_get_axis(transform_b, 1))) &&
		TEST_OVERLAP(plt_vector3f_cross(plt_matrix_get_axis(transform_a, 1), plt_matrix_get_axis(transform_b, 2))) &&
		TEST_OVERLAP(plt_vector3f_cross(plt_matrix_get_axis(transform_a, 2), plt_matrix_get_axis(transform_b, 0))) &&
		TEST_OVERLAP(plt_vector3f_cross(plt_matrix_get_axis(transform_a, 2), plt_matrix_get_axis(transform_b, 1))) &&
		TEST_OVERLAP(plt_vector3f_cross(plt_matrix_get_axis(transform_a, 2), plt_matrix_get_axis(transform_b, 2)))
	);
}
