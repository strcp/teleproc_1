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

int save_packet_fragment(struct iphdr *ip)
{
	struct fragment_list *flist, *fl;
	struct hash_frags *ht;
	struct data_info *tmp;
	int size;

	if (!ip)
		return 0;

	ht = malloc(sizeof(struct hash_frags));

	ht->index = ip->frag_off;
	tmp = get_packet_data((char *)ip);
	size = (sizeof(struct data_info) + tmp->name_size + 1 + tmp->data_size);
	ht->data = (struct data_info *)malloc(size);
	memcpy(ht->data, tmp, size);

	flist = malloc(sizeof(struct fragment_list));
	flist->pkt_id = ip->id;
	flist->frag = ht;
	flist->prev = NULL;
	flist->next = NULL;

	for (fl = frag_list; fl; fl = fl->next) {
		if (!fl->next) {
			fl->next = flist;
			flist->prev = fl;
		}
	}
	if (!frag_list)
		frag_list = flist;

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

void free_fragment_list(struct fragment_list *flist)
{
}
#endif
