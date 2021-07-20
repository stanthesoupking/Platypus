#include "platypus/platypus.h"

#include <stdlib.h>
#include "GLFW/glfw3.h"

#include "platypus/base/macros.h"

typedef struct Plt_Application {
	GLFWwindow *window;
} Plt_Application;

Plt_Application *plt_application_create(const char *title, Plt_Application_Option options) {
	Plt_Application *application = malloc(sizeof(Plt_Application));

	if (!glfwInit()) {
		plt_abort("GLFW init failed.");
	}

	application->window = glfwCreateWindow(860, 640, title, NULL, NULL);
	plt_assert(application->window, "Failed creating GLFW window.");

	return application;
}
void plt_application_destroy(Plt_Application **application) {
	glfwDestroyWindow((*application)->window);
	glfwTerminate();

	free(*application);
	*application = NULL;
}

void plt_application_show(Plt_Application *application) {
	glfwShowWindow(application->window);
}
bool plt_application_should_close(Plt_Application *application) {
	return glfwWindowShouldClose(application->window);
}
void plt_application_update(Plt_Application *application) {
	glfwPollEvents();
}
