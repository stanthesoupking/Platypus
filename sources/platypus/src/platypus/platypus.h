#ifndef PLATYPUS_H
#define PLATYPUS_H

#include <stdbool.h>

typedef enum Plt_Application_Option {
	Plt_Application_Option_None = 0
} Plt_Application_Option;

typedef struct Plt_Application Plt_Application;
Plt_Application *plt_application_create(const char *title, Plt_Application_Option options);
void plt_application_destroy(Plt_Application **application);

void plt_application_show(Plt_Application *application);
bool plt_application_should_close(Plt_Application *application);
void plt_application_update(Plt_Application *application);

#endif