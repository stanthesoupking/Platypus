#include <stdio.h>

#include "platypus/platypus.h"

int main(int argc, char **argv) {
	printf("Hello\n");

	Plt_Application *app = plt_application_create("Platypus - Spinning Cube", Plt_Application_Option_None);

	while (!plt_application_should_close(app)) {
		plt_application_update(app);
	}

	plt_application_destroy(&app);

	return 0;
}