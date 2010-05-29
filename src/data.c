#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <data.h>

long int file_size(FILE *fp)
{
	long size;

	fseek(fp , 0 , SEEK_END);
	size = ftell(fp);
	rewind(fp);

	return size;
}

struct data_info *load_data(char *file_path)
{
	FILE *fp;
	long len;
	struct data_info *dinfo;

	if (!(fp = fopen(file_path, "rb"))) {
		printf("Error opening file.\n");
		return NULL;
	}
	len = file_size(fp);
	dinfo = malloc(sizeof(struct data_info));
	/* TODO: Parsear o nome */
	dinfo->name = strdup(file_path);
	dinfo->data = malloc(len);
	fread(dinfo->data, len, 1, fp);
	fclose(fp);

	return dinfo;
}

void free_data_info(struct data_info *dinfo)
{
	if (!dinfo)
		return;
	if (dinfo->name)
		free(dinfo->name);
	if (dinfo->data)
		free(dinfo->data);

	free(dinfo);
}

void dump_data(struct data_info *dinfo)
{
	printf("Dump data info\n");
	printf("Name: %s\n", dinfo->name);
	printf("Size: %d\n\n", dinfo->size);
}
