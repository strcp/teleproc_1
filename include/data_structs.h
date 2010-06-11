/******************************************************************
 * Data : 17.06.2010
 * Disciplina   : Comunicação de dados e Teleprocessamento - PUCRS
 *
 * Autores  : Cristiano Bolla Fernandes
 *          : Benito Michelon
 *****************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

struct hash_frags
{
	int index;
	struct data_info *data;
};

struct fragment_list
{
	unsigned short pkt_id;
	struct hash_frags *frag;
	struct fragment_list *next;
	struct fragment_list *prev;
};

struct fragment_list *frag_list;

int save_packet_fragment(struct iphdr *ip);
struct data_info *get_defregmented_data(int ip_id);
void free_hash(struct hash_frags * ht);
void free_fragments(int id);
void free_fragment_list(struct fragment_list *flist);
