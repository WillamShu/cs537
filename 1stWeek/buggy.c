#include <stdio.h>

struct Data {
	int x;
};

int
main(int argc, char *argv[])
{
	struct Data *p = NULL;
	printf("%d\n", p->x);
}
