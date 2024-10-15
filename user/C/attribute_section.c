/*
 * @*************************************: 
 * @FilePath     : /user/C/attribute_section.c
 * @version      : 
 * @Author       : dof
 * @Date         : 2024-09-13 19:06:29
 * @LastEditors  : dof
 * @LastEditTime : 2024-09-13 19:10:55
 * @Descripttion :  
 * @compile      :  
 * @**************************************: 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// struct entry_st;
typedef XML_OPR_t (*xml_set_t)(struct entry_st *, const char *);
typedef XML_OPR_t (*xml_mcp_set_t)(struct entry_st *, const char *);
typedef XML_OPR_t (*xml_get_t)(struct entry_st *, char *, unsigned int *byte_count);
typedef XML_OPR_t (*xml_mcp_get_t)(struct entry_st *, char *, unsigned int *byte_count);
typedef void (*xml_init_index_t) (void);
typedef XML_OPR_t (*xml_dir_visit_t) (void);
typedef int (*xml_dir_index_t) (void);


typedef struct chain_st {
	struct chain_st *pNext;
	unsigned char   *pData;
} chain_t;


typedef struct entry_st {
	struct dir_st *dir;
	struct entry_st *sibling;
	const char *name;
	const char *matchname;

	// setting
	xml_set_t set;
	xml_mcp_set_t mcp_set;

	// getting
	xml_get_t get;
	xml_mcp_get_t mcp_get;

	// initial index
	xml_init_index_t init_index;
} entry_t;


typedef struct dir_st {
	struct dir_st *parent;
	struct dir_st *sibling;
	struct dir_st *firstChild;
	const char *name;
	const char *matchname;
	struct entry_st *firstEntry;
	int id;             // For chain id
	unsigned int index;
	unsigned int final_idx;      // Final available index number of DIR
	unsigned int index_max;      // Define the max index number
	int empty_chain;    // Flag to identify the chain without entry
	chain_t headNode;
	unsigned char **ppData;
	unsigned int size;
} dir_t;



#define XML_DIR( \
	p, \
	n, \
	dn, \
	max, \
	Id	\
	) \
	dir_t n##_dir \
	= { \
		.name=#n, \
		.matchname=dn, \
		.parent=&p##_dir, \
		.index     = 0, \
		.final_idx = 0, \
		.index_max = max, \
		.id=Id, \
		.headNode  = {.pNext = 0, .pData = 0}, \
		.ppData    = (unsigned char **)&n,  \
		.size      = sizeof(*n)  \
	},	\
	*n##_dir_p	\
	__attribute__((aligned(4), unused, __section__(".dir_list")))	\
	= &n##_dir


#define XML_DIR_ARRAY(p, n, dn, max, Id)  XML_DIR(p, n, dn, max, Id)


#define MIB_QOS_TAB (302)

typedef struct
{
    uword32 ulStateAndIndex;
    uword8 ucEnable;

#define QOS_ATTR_MASK_BIT1_QOS_ENABLE       (0x01)
#define QOS_ATTR_MASK_ALL (0xf)

    uword32 ulBitmap;
} __PACK__ IgdQosAttrConfTab;


typedef IgdQosAttrConfTab MIB_QOS_T, *MIB_QOS_Tp;

MIB_QOS_Tp em_qos_entry = 0;
XML_DIR_ARRAY( root, em_qos_entry, "EM_QOS_TAB", 1, MIB_QOS_TAB);
// XML_ENTRY_PRIMITIVE2(em_qos_entry, ulStateAndIndex);
// XML_ENTRY_UCHAR2(em_qos_entry, ucEnable);
// XML_ENTRY_PRIMITIVE2(em_qos_entry, ulBitmap);



int main(int argc, char const *argv[])
{
    
    return 0;
}
