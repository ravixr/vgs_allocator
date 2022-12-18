#include <stdio.h>
#include "vgs_allocator.h"
#include <stdlib.h>
#include <string.h>

int main()
{
	char *str = vgs_malloc(sizeof(char) * 32);
	int *arr = vgs_malloc(sizeof(int) * 10);
	char *str2 = vgs_malloc(sizeof(char) * 32);
	char *str3 = vgs_malloc(sizeof(char) * 32);
	vgs_free(arr);
	vgs_free(str2);
	vgs_free(str3);
	char *str4 = vgs_malloc(sizeof(char) * 64);
	char *str5 = vgs_calloc(sizeof(char) * 100);
	str = vgs_realloc(str, 72);
	vgs_free(str5);
	strcpy(str4, "hello\n");
	puts(str4);
}