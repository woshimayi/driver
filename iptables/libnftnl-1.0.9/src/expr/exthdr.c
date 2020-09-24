/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include "internal.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <arpa/inet.h>
#include <errno.h>
#include <libmnl/libmnl.h>

#include <linux/netfilter/nf_tables.h>

#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

#ifndef IPPROTO_MH
#define IPPROTO_MH 135
#endif

struct nftnl_expr_exthdr {
	enum nft_registers	dreg;
	enum nft_registers	sreg;
	uint32_t		offset;
	uint32_t		len;
	uint8_t			type;
	uint32_t		op;
	uint32_t		flags;
};

static int
nftnl_expr_exthdr_set(struct nftnl_expr *e, uint16_t type,
			  const void *data, uint32_t data_len)
{
	struct nftnl_expr_exthdr *exthdr = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_EXTHDR_DREG:
		exthdr->dreg = *((uint32_t *)data);
		break;
	case NFTNL_EXPR_EXTHDR_TYPE:
		exthdr->type = *((uint8_t *)data);
		break;
	case NFTNL_EXPR_EXTHDR_OFFSET:
		exthdr->offset = *((uint32_t *)data);
		break;
	case NFTNL_EXPR_EXTHDR_LEN:
		exthdr->len = *((uint32_t *)data);
		break;
	case NFTNL_EXPR_EXTHDR_OP:
		exthdr->op = *((uint32_t *)data);
		break;
	case NFTNL_EXPR_EXTHDR_FLAGS:
		exthdr->flags = *((uint32_t *)data);
		break;
	case NFTNL_EXPR_EXTHDR_SREG:
		exthdr->sreg = *((uint32_t *)data);
		break;
	default:
		return -1;
	}
	return 0;
}

static const void *
nftnl_expr_exthdr_get(const struct nftnl_expr *e, uint16_t type,
			 uint32_t *data_len)
{
	struct nftnl_expr_exthdr *exthdr = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_EXTHDR_DREG:
		*data_len = sizeof(exthdr->dreg);
		return &exthdr->dreg;
	case NFTNL_EXPR_EXTHDR_TYPE:
		*data_len = sizeof(exthdr->type);
		return &exthdr->type;
	case NFTNL_EXPR_EXTHDR_OFFSET:
		*data_len = sizeof(exthdr->offset);
		return &exthdr->offset;
	case NFTNL_EXPR_EXTHDR_LEN:
		*data_len = sizeof(exthdr->len);
		return &exthdr->len;
	case NFTNL_EXPR_EXTHDR_OP:
		*data_len = sizeof(exthdr->op);
		return &exthdr->op;
	case NFTNL_EXPR_EXTHDR_FLAGS:
		*data_len = sizeof(exthdr->flags);
		return &exthdr->flags;
	case NFTNL_EXPR_EXTHDR_SREG:
		*data_len = sizeof(exthdr->sreg);
		return &exthdr->sreg;
	}
	return NULL;
}

static int nftnl_expr_exthdr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_EXTHDR_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_EXTHDR_TYPE:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			abi_breakage();
		break;
	case NFTA_EXTHDR_DREG:
	case NFTA_EXTHDR_SREG:
	case NFTA_EXTHDR_OFFSET:
	case NFTA_EXTHDR_LEN:
	case NFTA_EXTHDR_OP:
	case NFTA_EXTHDR_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_exthdr_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_exthdr *exthdr = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_DREG))
		mnl_attr_put_u32(nlh, NFTA_EXTHDR_DREG, htonl(exthdr->dreg));
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_SREG))
		mnl_attr_put_u32(nlh, NFTA_EXTHDR_SREG, htonl(exthdr->sreg));
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_TYPE))
		mnl_attr_put_u8(nlh, NFTA_EXTHDR_TYPE, exthdr->type);
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_OFFSET))
		mnl_attr_put_u32(nlh, NFTA_EXTHDR_OFFSET, htonl(exthdr->offset));
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_LEN))
		mnl_attr_put_u32(nlh, NFTA_EXTHDR_LEN, htonl(exthdr->len));
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_OP))
		mnl_attr_put_u32(nlh, NFTA_EXTHDR_OP, htonl(exthdr->op));
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_EXTHDR_FLAGS, htonl(exthdr->flags));
}

static int
nftnl_expr_exthdr_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_exthdr *exthdr = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_EXTHDR_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_exthdr_cb, tb) < 0)
		return -1;

	if (tb[NFTA_EXTHDR_DREG]) {
		exthdr->dreg = ntohl(mnl_attr_get_u32(tb[NFTA_EXTHDR_DREG]));
		e->flags |= (1 << NFTNL_EXPR_EXTHDR_DREG);
	}
	if (tb[NFTA_EXTHDR_SREG]) {
		exthdr->sreg = ntohl(mnl_attr_get_u32(tb[NFTA_EXTHDR_SREG]));
		e->flags |= (1 << NFTNL_EXPR_EXTHDR_SREG);
	}
	if (tb[NFTA_EXTHDR_TYPE]) {
		exthdr->type = mnl_attr_get_u8(tb[NFTA_EXTHDR_TYPE]);
		e->flags |= (1 << NFTNL_EXPR_EXTHDR_TYPE);
	}
	if (tb[NFTA_EXTHDR_OFFSET]) {
		exthdr->offset = ntohl(mnl_attr_get_u32(tb[NFTA_EXTHDR_OFFSET]));
		e->flags |= (1 << NFTNL_EXPR_EXTHDR_OFFSET);
	}
	if (tb[NFTA_EXTHDR_LEN]) {
		exthdr->len = ntohl(mnl_attr_get_u32(tb[NFTA_EXTHDR_LEN]));
		e->flags |= (1 << NFTNL_EXPR_EXTHDR_LEN);
	}
	if (tb[NFTA_EXTHDR_OP]) {
		exthdr->op = ntohl(mnl_attr_get_u32(tb[NFTA_EXTHDR_OP]));
		e->flags |= (1 << NFTNL_EXPR_EXTHDR_OP);
	}
	if (tb[NFTA_EXTHDR_FLAGS]) {
		exthdr->flags = ntohl(mnl_attr_get_u32(tb[NFTA_EXTHDR_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_EXTHDR_FLAGS);
	}

	return 0;
}

static const char *op2str(uint8_t op)
{
	switch (op) {
	case NFT_EXTHDR_OP_TCPOPT:
		return " tcpopt";
	case NFT_EXTHDR_OP_IPV6:
	default:
		return "";
	}
}

static const char *type2str(uint32_t type)
{
	switch (type) {
	case IPPROTO_HOPOPTS:
		return "hopopts";
	case IPPROTO_ROUTING:
		return "routing";
	case IPPROTO_FRAGMENT:
		return "fragment";
	case IPPROTO_DSTOPTS:
		return "dstopts";
	case IPPROTO_MH:
		return "mh";
	default:
		return "unknown";
	}
}

static inline int str2exthdr_op(const char* str)
{
	if (!strcmp(str, "tcpopt"))
		return NFT_EXTHDR_OP_TCPOPT;

	/* if str == "ipv6" or anything else */
	return NFT_EXTHDR_OP_IPV6;
}

static inline int str2exthdr_type(const char *str)
{
	if (strcmp(str, "hopopts") == 0)
		return IPPROTO_HOPOPTS;
	else if (strcmp(str, "routing") == 0)
		return IPPROTO_ROUTING;
	else if (strcmp(str, "fragment") == 0)
		return IPPROTO_FRAGMENT;
	else if (strcmp(str, "dstopts") == 0)
		return IPPROTO_DSTOPTS;
	else if (strcmp(str, "mh") == 0)
		return IPPROTO_MH;

	return -1;
}

static int
nftnl_expr_exthdr_json_parse(struct nftnl_expr *e, json_t *root,
				struct nftnl_parse_err *err)
{
#ifdef JSON_PARSING
	const char *exthdr_type;
	uint32_t uval32;
	int type;

	if (nftnl_jansson_parse_reg(root, "dreg", NFTNL_TYPE_U32, &uval32,
				  err) == 0)
		nftnl_expr_set_u32(e, NFTNL_EXPR_EXTHDR_DREG, uval32);

	if (nftnl_jansson_parse_reg(root, "sreg", NFTNL_TYPE_U32, &uval32,
				  err) == 0)
		nftnl_expr_set_u32(e, NFTNL_EXPR_EXTHDR_SREG, uval32);

	exthdr_type = nftnl_jansson_parse_str(root, "exthdr_type", err);
	if (exthdr_type != NULL) {
		type = str2exthdr_type(exthdr_type);
		if (type < 0)
			return -1;
		nftnl_expr_set_u32(e, NFTNL_EXPR_EXTHDR_TYPE, type);
	}

	if (nftnl_jansson_parse_val(root, "offset", NFTNL_TYPE_U32, &uval32,
				  err) == 0)
		nftnl_expr_set_u32(e, NFTNL_EXPR_EXTHDR_OFFSET, uval32);

	if (nftnl_jansson_parse_val(root, "len", NFTNL_TYPE_U32, &uval32, err) == 0)
		nftnl_expr_set_u32(e, NFTNL_EXPR_EXTHDR_LEN, uval32);

	if (nftnl_jansson_parse_val(root, "op", NFTNL_TYPE_U32, &uval32, err) == 0)
		nftnl_expr_set_u32(e, NFTNL_EXPR_EXTHDR_OP, uval32);

	if (nftnl_jansson_parse_val(root, "flags", NFTNL_TYPE_U32, &uval32, err) == 0)
		nftnl_expr_set_u32(e, NFTNL_EXPR_EXTHDR_FLAGS, uval32);

	return 0;
#else
	errno = EOPNOTSUPP;
	return -1;
#endif
}

static int nftnl_expr_exthdr_export(char *buf, size_t len,
				    const struct nftnl_expr *e, int type)
{
	struct nftnl_expr_exthdr *exthdr = nftnl_expr_data(e);
	NFTNL_BUF_INIT(b, buf, len);

	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_DREG))
		nftnl_buf_u32(&b, type, exthdr->dreg, DREG);
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_SREG))
		nftnl_buf_u32(&b, type, exthdr->dreg, SREG);
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_TYPE))
		nftnl_buf_str(&b, type, type2str(exthdr->type), EXTHDR_TYPE);
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_OFFSET))
		nftnl_buf_u32(&b, type, exthdr->offset, OFFSET);
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_LEN))
		nftnl_buf_u32(&b, type, exthdr->len, LEN);
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_OP))
		nftnl_buf_u32(&b, type, exthdr->op, OP);
	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_FLAGS))
		nftnl_buf_u32(&b, type, exthdr->flags, FLAGS);

	return nftnl_buf_done(&b);
}

static int nftnl_expr_exthdr_snprintf_default(char *buf, size_t len,
					      const struct nftnl_expr *e)
{
	struct nftnl_expr_exthdr *exthdr = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_EXTHDR_DREG))
		return snprintf(buf, len, "load%s %ub @ %u + %u%s => reg %u ",
				op2str(exthdr->op), exthdr->len, exthdr->type,
				exthdr->offset,
				exthdr->flags & NFT_EXTHDR_F_PRESENT ? " present" : "",
				exthdr->dreg);
	else
		return snprintf(buf, len, "write%s reg %u => %ub @ %u + %u ",
				op2str(exthdr->op), exthdr->sreg, exthdr->len, exthdr->type,
				exthdr->offset);

}

static int
nftnl_expr_exthdr_snprintf(char *buf, size_t len, uint32_t type,
			   uint32_t flags, const struct nftnl_expr *e)
{
	switch (type) {
	case NFTNL_OUTPUT_DEFAULT:
		return nftnl_expr_exthdr_snprintf_default(buf, len, e);
	case NFTNL_OUTPUT_XML:
	case NFTNL_OUTPUT_JSON:
		return nftnl_expr_exthdr_export(buf, len, e, type);
	default:
		break;
	}
	return -1;
}

static bool nftnl_expr_exthdr_cmp(const struct nftnl_expr *e1,
				  const struct nftnl_expr *e2)
{
	struct nftnl_expr_exthdr *h1 = nftnl_expr_data(e1);
	struct nftnl_expr_exthdr *h2 = nftnl_expr_data(e2);
	bool eq = true;

	if (e1->flags & (1 << NFTNL_EXPR_EXTHDR_DREG))
		eq &= (h1->dreg == h2->dreg);
	if (e1->flags & (1 << NFTNL_EXPR_EXTHDR_SREG))
		eq &= (h1->sreg == h2->sreg);
	if (e1->flags & (1 << NFTNL_EXPR_EXTHDR_OFFSET))
		eq &= (h1->offset == h2->offset);
	if (e1->flags & (1 << NFTNL_EXPR_EXTHDR_LEN))
		eq &= (h1->len == h2->len);
	if (e1->flags & (1 << NFTNL_EXPR_EXTHDR_TYPE))
		eq &= (h1->type == h2->type);
	if (e1->flags & (1 << NFTNL_EXPR_EXTHDR_OP))
		eq &= (h1->op == h2->op);
	if (e1->flags & (1 << NFTNL_EXPR_EXTHDR_FLAGS))
		eq &= (h1->flags == h2->flags);

	return eq;
}

struct expr_ops expr_ops_exthdr = {
	.name		= "exthdr",
	.alloc_len	= sizeof(struct nftnl_expr_exthdr),
	.max_attr	= NFTA_EXTHDR_MAX,
	.cmp		= nftnl_expr_exthdr_cmp,
	.set		= nftnl_expr_exthdr_set,
	.get		= nftnl_expr_exthdr_get,
	.parse		= nftnl_expr_exthdr_parse,
	.build		= nftnl_expr_exthdr_build,
	.snprintf	= nftnl_expr_exthdr_snprintf,
	.json_parse	= nftnl_expr_exthdr_json_parse,
};
