#include <stdio.h>
#include <stdlib.h>

int cmp(const void *a, const void *b) {
	return *(int*)a - *(int*)b;
}

int main(int argc, char const *argv[])
{
	int tosort[] = {2, 1, 3};
	qsort(tosort, 3, sizeof(int), cmp);
	printf("%d %d %d", tosort[0], tosort[1], tosort[2]);
	return 0;
}