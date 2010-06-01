/******************************************************************
 * Data : 17.06.201
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

#include <stdio.h>

/**
 * @brief Data Information
 */
struct data_info
{
	/*@{*/
	long int size;		/**< Tamanho do arquivo carregado */
	char name[255];		/**< Nome do arquivo */
	void *data;			/**< Ponteiro para os dados do arquivo carregados */
	/*@}*/
};

long int file_size(FILE *fp);
struct data_info *load_data(char *file_path);
void dump_data(struct data_info *dinfo);
void free_data_info(struct data_info *dinfo);
int save_data(struct data_info *data);

