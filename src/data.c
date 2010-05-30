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
	printf("Size: %ld\n", size);

	return size;
}

struct data_info *load_data(char *file_path)
{
	FILE *fp;
	long len;
	struct data_info *dinfo;
	char *tmp;

	if (!(fp = fopen(file_path, "rb"))) {
		printf("Error opening file.\n");
		return NULL;
	}
	if ((len = file_size(fp)) < 0) {
		printf("Error calculating file size.\n");
		return NULL;
	}
	dinfo = malloc(sizeof(struct data_info));
	memset(dinfo, 0, sizeof(struct data_info));
	if ((tmp = strrchr(file_path, '/')))
		snprintf(dinfo->name, 255, ++(tmp));
	else
		snprintf(dinfo->name, 255, file_path);
	dinfo->data = malloc(len);
	dinfo->size = len;
	fread(dinfo->data, len, 1, fp);
	fclose(fp);

	return dinfo;
}

int save_data(struct data_info *data)
{
	FILE *fp;

	if (!(fp = fopen(data->name, "wb"))) {
		printf("Error opening %s\n", data->name);
		return -1;
	}
	if (!fwrite(data->data, data->size, 1, fp)) {
		printf("Error writing file %s\n", data->name);
		return -1;
	}
	//TODO: Definir esse retorno, talvez retornar o retorno de fwrite()
	return 0;
}

void free_data_info(struct data_info *dinfo)
{
	if (!dinfo)
		return;
	if (dinfo->data)
		free(dinfo->data);

	free(dinfo);
}

void dump_data(struct data_info *dinfo)
{
	printf("Dump data info\n");
	printf("Name: %s\n", dinfo->name);
	printf("Size: %ld\n\n", dinfo->size);
}
