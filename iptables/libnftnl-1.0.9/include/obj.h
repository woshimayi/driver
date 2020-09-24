#ifndef _OBJ_OPS_H_
#define _OBJ_OPS_H_

#include <stdint.h>
#include "internal.h"

struct nlattr;
struct nlmsghdr;
struct nftnl_obj;

struct nftnl_obj {
	struct list_head	head;
	struct obj_ops		*ops;

	const char		*table;
	const char		*name;

	uint32_t		family;
	uint32_t		use;

	uint32_t		flags;

	union {
		struct nftnl_obj_counter {
			uint64_t	pkts;
			uint64_t	bytes;
		} counter;
		struct nftnl_obj_quota {
			uint64_t        bytes;
			uint64_t	consumed;
			uint32_t        flags;
		} quota;
		struct nftnl_obj_ct_helper {
			uint16_t	l3proto;
			uint8_t		l4proto;
			char		name[16];
		} ct_helper;
		struct nftnl_obj_limit {
			uint64_t	rate;
			uint64_t	unit;
			uint32_t	burst;
			uint32_t	type;
			uint32_t	flags;
		} limit;
	} data;
};

struct obj_ops {
	const char *name;
	uint32_t type;
	size_t	alloc_len;
	int	max_attr;
	int	(*set)(struct nftnl_obj *e, uint16_t type, const void *data, uint32_t data_len);
	const void *(*get)(const struct nftnl_obj *e, uint16_t type, uint32_t *data_len);
	int	(*parse)(struct nftnl_obj *e, struct nlattr *attr);
	void	(*build)(struct nlmsghdr *nlh, const struct nftnl_obj *e);
	int	(*snprintf)(char *buf, size_t len, uint32_t type, uint32_t flags, const struct nftnl_obj *e);
	int	(*json_parse)(struct nftnl_obj *e, json_t *data,
			      struct nftnl_parse_err *err);
};

extern struct obj_ops obj_ops_counter;
extern struct obj_ops obj_ops_quota;
extern struct obj_ops obj_ops_ct_helper;
extern struct obj_ops obj_ops_limit;

#define nftnl_obj_data(obj) (void *)&obj->data

#endif
