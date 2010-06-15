/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include <data_structs.h>
#include <data.h>
#include <connection.h>


struct fragment_list *frag_list = NULL;


static void free_frag_list(struct fragment_list *list)
{
	struct fragment_list *flist, *f;

	if (!list)
		return;

	for (flist = list; flist; f = flist, flist = flist->next) {
		if (f->frag)
			free_data_info(f->frag);
		free(f);
		f = NULL;
	}
}

struct fragment_list *sort_fragments(struct fragment_list *flist)
{
	struct fragment_list *f, *tmp;

	f = flist;

	while (f) {
		if (!f->prev || f->prev->frag->seq <= f->frag->seq) {
			f = f->next;
		} else {
		/*	tmp = f->next;
			f->next->prev = f;
			f->next->next = tmp;

			f->next = f->prev;
			f->prev = f->next->prev;
		*/
			f->prev->next = f->next;
			if (f->next)
				f->next->prev = f->prev;
			tmp = f->prev->prev;
			f->prev->prev = f;
			f->next = f->prev;
			f->prev = tmp;
			if (tmp){
				tmp->next = f;
			} else 
				flist = f;
		}
	}
	for (f = flist; f; f = f->next) {
		printf("%d\t", f->frag->seq);
	}
	printf("\n");
	return flist;
}

struct fragment_list *list_prepend(struct fragment_list *flist, struct data_info *dinfo)
{
	struct fragment_list *list;

	list = malloc(sizeof(struct fragment_list));
	list->frag = dinfo;
	list->prev = NULL;
	list->next = NULL;
	if (flist) {
		list->next = flist;
		flist->prev = list;
	}

	return list;
}

struct fragment_list *fragment_packet(void *data)
{
	struct data_info *frags;
	struct fragment_list *flist;
	long int i, seq, pkt_size, data_size;
	char *offset;

	struct data_info *dinfo = (struct data_info *)data;


	if (!data)
		return NULL;

	offset = (char *)data + sizeof(struct data_info);
	data_size = MAX_DATA_SIZE - sizeof(struct data_info);

	flist = NULL;

	i = (dinfo->tot_len - sizeof(struct data_info));
	for (seq = 0; i > 0; i -= data_size, seq++) {

		pkt_size = (i - data_size) >= 0 ? data_size : i;
		pkt_size += sizeof(struct data_info);

		frags = malloc(pkt_size);

		frags->tot_len = pkt_size;
		frags->seq = seq;
		frags->name_size = 0;
		frags->id = dinfo->id;
		if (!seq)
			frags->name_size = dinfo->name_size;

		frags->data_size = frags->tot_len - (sizeof(struct data_info) + frags->name_size + 1);

		if ((i - data_size) <= 0)
			/* Último fragmento */
			frags->fragmented = 2;
		else
			frags->fragmented = 1;

		memcpy((char *)frags + sizeof(struct data_info), offset, frags->tot_len - sizeof(struct data_info));
		offset += (frags->tot_len - sizeof(struct data_info));
		flist = list_prepend(flist, frags);
	}
	return flist;
}

/* Defragmenta um dado e retorna como uma única estrutura */
/* TODO */
struct data_info *get_defragmented_data(int id)
{
	struct data_info *dinfo;
	struct fragment_list *f, *frags;
	long int size;
	char *data;

	size = 0;
	frags = get_frag_id_list(id);
	frags = sort_fragments(frags);

	for (f = frags; f; f = f->next) {
		printf("DEBUGSEQ: %d\n", f->frag->seq);
		if (f->frag->seq == 0)
			printf("DEBUGNOME: %s\n", (char *)f->frag + sizeof(struct data_info));
		size += f->frag->tot_len - sizeof(struct data_info);
	}
	dinfo = malloc(size + sizeof(struct data_info));
	dinfo->tot_len = size + sizeof(struct data_info);

	data = (char *)dinfo + sizeof(struct data_info);
	for (f = frags; f; f = f->next) {
		memcpy(data, (char *)f->frag + sizeof(struct data_info), f->frag->tot_len - sizeof(struct data_info));
		data = data + (f->frag->tot_len - sizeof(struct data_info));
		dinfo->data_size += f->frag->tot_len - sizeof(struct data_info);
		if (f->frag->seq == 0) {
			dinfo->name_size = f->frag->name_size;
			dinfo->id = f->frag->id;
		}
	}
	dinfo->data_size -= (dinfo->name_size + 1);	/* Não esquecendo do \0 :-) */
	dinfo->seq = 0;
	dinfo->fragmented = 0;
	/* FIXME: Arrumar libreação de memoria */
	// free_frag_list(frags);

	return dinfo;
}

/* Salva o fragmento no buffer global */
int save_packet_fragment(struct data_info *dinfo)
{
	if ((frag_list = list_prepend(frag_list, dinfo)))
		return 1;
	return 0;
}

struct fragment_list *list_steal(struct fragment_list *node)
{
	if (node->next)
		node->next->prev = node->prev;
	if (node->prev)
		node->prev->next = node->next;

	node->next = NULL;
	node->prev = NULL;
	return node;
}

/* Retorna uma lista relativa ao fragmento de id */
struct fragment_list *get_frag_id_list(int id)
{
	struct fragment_list *f, *seq_list, *aux;

	seq_list = NULL;
	aux = NULL;
	for (f = frag_list; f; f = aux) {
		aux = f->next;
		if (f->frag->id == id) {
			f = list_steal(f);
			if (f == frag_list)
				frag_list = frag_list->next ? frag_list->next : NULL;
			seq_list = list_prepend(seq_list, f->frag);
			free(f);
			f = NULL;
		}
	}

	return seq_list;
}

int fragment_list_length(struct fragment_list *flist)
{
	struct fragment_list *f;
	int n = 0;

	for (f = flist; f; f = f->next) {
		n++;
	}

	return n;
}

/* Verifica se o pacote está completo */
int is_packet_complete(struct data_info *dinfo)
{
	struct fragment_list *f;
	int last, n;

	for (n = 0, last = -2, f = frag_list; f; f = f->next) {
		if (f->frag->id == dinfo->id) {
			n++;
			if (f->frag->fragmented == 2)
				last = f->frag->seq;
		}
	}

	/* n - 1 == last? já que os números de seq começam em 0 */
	return  (n - 1) == last ? 1 : 0;
}

void cleanup_frag_list(void)
{
	free_frag_list(frag_list);
}
