#include "platypus/platypus.h"

#include <stdlib.h>
#include "platypus/glad/glad.h"
#include "GLFW/glfw3.h"

#include "platypus/base/macros.h"

#define FRAMEBUFFER_WIDTH 320
#define FRAMEBUFFER_HEIGHT 240

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static const char* vertex_shader_text =
"#version 330\n"
"in vec4 v_in;\n"
"out vec2 o_uv;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(v_in.xy, 0.0, 1.0);\n"
"	o_uv = v_in.zw;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 330\n"
"in vec4 v_in;\n"
"in vec2 o_uv;\n"
"uniform usampler2D display_texture;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"	frag_color = texture(display_texture, o_uv);\n"
"}\n";

typedef struct Plt_Application_GL_Data {
	GLuint program, vertex_buffer, vao, display_texture;
} Plt_Application_GL_Data;

typedef struct Plt_Application {
	GLFWwindow *window;
	Plt_Application_GL_Data gl_data;

	Plt_Color8 *framebuffer;
} Plt_Application;

void plt_application_gl_init(Plt_Application *application);
GLuint plt_application_gl_create_program();
void plt_application_gl_present(Plt_Application *application);

Plt_Application *plt_application_create(const char *title, Plt_Application_Option options) {
	Plt_Application *application = malloc(sizeof(Plt_Application));

	application->framebuffer = malloc(sizeof(Plt_Color8) * FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);

	glfwSetErrorCallback(glfw_error_callback);

	if (!glfwInit()) {
		plt_abort("GLFW init failed.");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	application->window = glfwCreateWindow(860, 640, title, NULL, NULL);
	plt_assert(application->window, "Failed creating GLFW window.");

	glfwMakeContextCurrent(application->window);
    glfwSwapInterval(1);

	plt_application_gl_init(application);

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

	// Draw pixel
	application->framebuffer[10 * FRAMEBUFFER_WIDTH + 10].r = 255;

	plt_application_gl_present(application);
}

void plt_application_gl_init(Plt_Application *application) {
	gladLoadGL();

	glGenVertexArrays(1, &(application->gl_data.vao));
	glBindVertexArray(application->gl_data.vao);

	// Create vertex buffer
	{
		float vertices[16] = {
			-1, -1, 0, 1,
			 1, -1, 1, 1,
			-1,  1, 0, 0,
			 1,  1, 1, 0,
		};
		glGenBuffers(1, &(application->gl_data.vertex_buffer));
		glBindBuffer(GL_ARRAY_BUFFER, application->gl_data.vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, vertices, GL_STATIC_DRAW);
	}

	application->gl_data.program = plt_application_gl_create_program();
	glUseProgram(application->gl_data.program);
 
	GLuint v_in_location = glGetAttribLocation(application->gl_data.program, "v_in");
    glVertexAttribPointer(v_in_location, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
	glEnableVertexAttribArray(v_in_location);

	glGenTextures(1, &(application->gl_data.display_texture));
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, application->gl_data.display_texture);
}

GLuint plt_application_gl_create_program() {
	int success;
	char infolog[512];
	
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex_shader, 512, NULL, infolog);
		printf("Vertex shader compilation failed:\n%s\n", infolog);
	}
 
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);
 
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);

	glAttachShader(program, fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment_shader, 512, NULL, infolog);
		printf("Fragment shader compilation failed:\n%s\n", infolog);
	}

	glLinkProgram(program);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, NULL, infolog);
		printf("Shader compilation failed:\n%s\n", infolog);
	}
	
	return program;
}
void plt_application_gl_present(Plt_Application *application) {
	// Update viewport
	{
		int width, height;
		glfwGetFramebufferSize(application->window, &width, &height);
		glViewport(0, 0, width, height);
	}

	// Update display texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, application->framebuffer);

	// Render
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	glfwSwapBuffers(application->window);
}