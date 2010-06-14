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


struct fragment_list *frag_list;

struct fragment_list *sort_fragments(struct fragment_list *flist)
{
	struct fragment_list *f, *tmp;

	f = flist;

	while (f) {
		if (!f->prev || f->prev->frag->seq <= f->frag->seq) {
			f = f->next;
		} else {
			tmp = f->next;
			f->next = f->prev;
			f->prev = f->next->prev;

			f->next->prev = f;
			f->next->next = tmp;
		}
	}

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
	struct data_info *dinfo = (struct data_info *)data;
	struct data_info *frags;
	struct fragment_list *flist;
	long int i, seq, pkt_size, data_size;
	char *offset;


	if (!data)
		return NULL;

	offset = (char *)data + sizeof(struct data_info);
	data_size = MAX_DATA_SIZE - sizeof(struct data_info);

	flist = NULL;

	i = (dinfo->tot_len - sizeof(struct data_info));
	for (seq = 0; i > 0; i -= data_size, seq++) {

		pkt_size = (i - data_size) >= 0 ? data_size : i;
		printf("Size: %ld\n", pkt_size);
		pkt_size += sizeof(struct data_info);

		frags = malloc(pkt_size);

		frags->tot_len = pkt_size;
		frags->seq = seq;
		frags->name_size = 0;
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

struct data_info *get_defregmented_data(struct fragment_list *frags)
{
	struct data_info *dinfo;
	struct fragment_list *f;

	frags = sort_fragments(frags);

	for (f = frags; f; f = f->next) {
		printf("seq: %d\n", f->frag->seq);
	}

	return NULL;
}

int save_packet_fragment(struct data_info *dinfo)
{
	struct fragment_list *flist;

	flist = malloc(sizeof(struct fragment_list));
	flist->prev = NULL;
	flist->next = NULL;
	flist->frag = dinfo;
	if (frag_list)
		flist->next = frag_list;

	return 1;
}

#if 0
struct data_info *get_defregmented_data(int ip_id)
{
}

void free_hash(struct hash_table * ht)
{
}

void free_fragments(int id)
{
}
#endif

void cleanup_frag_list(void)
{
	struct fragment_list *flist;

	for (flist = frag_list; flist; flist = flist->next) {
		if (flist->frag)
			free_data_info(flist->frag);
		free(flist);
	}
}
