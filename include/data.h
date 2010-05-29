#include <stdio.h>

struct data_info
{
	char *name;
	size_t size;
	void *data;
};

long int file_size(FILE *fp);
struct data_info *load_data(char *file_path);
void dump_data(struct data_info *dinfo);
void free_data_info(struct data_info *dinfo);