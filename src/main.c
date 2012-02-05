#include <gtk/gtk.h>
#include "interface.h"
#include "util.h"

// TODO: Is the actual output function always used ?!?!?

int main(int argc, char *argv[])
{
	gtk_init(&argc, &argv);

	menuMain();

	return 0;
}
