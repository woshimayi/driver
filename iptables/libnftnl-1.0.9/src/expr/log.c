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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter/nf_log.h>

#include "internal.h"
#include <libmnl/libmnl.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_log {
	uint32_t		snaplen;
	uint16_t		group;
	uint16_t		qthreshold;
	uint32_t		level;
	uint32_t		flags;
	const char		*prefix;
};

static int nftnl_expr_log_set(struct nftnl_expr *e, uint16_t type,
				 const void *data, uint32_t data_len)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_LOG_PREFIX:
		if (log->flags & (1 << NFTNL_EXPR_LOG_PREFIX))
			xfree(log->prefix);

		log->prefix = strdup(data);
		if (!log->prefix)
			return -1;
		break;
	case NFTNL_EXPR_LOG_GROUP:
		log->group = *((uint16_t *)data);
		break;
	case NFTNL_EXPR_LOG_SNAPLEN:
		log->snaplen = *((uint32_t *)data);
		break;
	case NFTNL_EXPR_LOG_QTHRESHOLD:
		log->qthreshold = *((uint16_t *)data);
		break;
	case NFTNL_EXPR_LOG_LEVEL:
		log->level = *((uint32_t *)data);
		break;
	case NFTNL_EXPR_LOG_FLAGS:
		log->flags = *((uint32_t *)data);
		break;
	default:
		return -1;
	}
	return 0;
}

static const void *
nftnl_expr_log_get(const struct nftnl_expr *e, uint16_t type,
		      uint32_t *data_len)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_LOG_PREFIX:
		*data_len = strlen(log->prefix)+1;
		return log->prefix;
	case NFTNL_EXPR_LOG_GROUP:
		*data_len = sizeof(log->group);
		return &log->group;
	case NFTNL_EXPR_LOG_SNAPLEN:
		*data_len = sizeof(log->snaplen);
		return &log->snaplen;
	case NFTNL_EXPR_LOG_QTHRESHOLD:
		*data_len = sizeof(log->qthreshold);
		return &log->qthreshold;
	case NFTNL_EXPR_LOG_LEVEL:
		*data_len = sizeof(log->level);
		return &log->level;
	case NFTNL_EXPR_LOG_FLAGS:
		*data_len = sizeof(log->flags);
		return &log->flags;
	}
	return NULL;
}

static int nftnl_expr_log_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_LOG_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_LOG_PREFIX:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			abi_breakage();
		break;
	case NFTA_LOG_GROUP:
	case NFTA_LOG_QTHRESHOLD:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			abi_breakage();
		break;
	case NFTA_LOG_SNAPLEN:
	case NFTA_LOG_LEVEL:
	case NFTA_LOG_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_log_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_LOG_PREFIX))
		mnl_attr_put_strz(nlh, NFTA_LOG_PREFIX, log->prefix);
	if (e->flags & (1 << NFTNL_EXPR_LOG_GROUP))
		mnl_attr_put_u16(nlh, NFTA_LOG_GROUP, htons(log->group));
	if (e->flags & (1 << NFTNL_EXPR_LOG_SNAPLEN))
		mnl_attr_put_u32(nlh, NFTA_LOG_SNAPLEN, htonl(log->snaplen));
	if (e->flags & (1 << NFTNL_EXPR_LOG_QTHRESHOLD))
		mnl_attr_put_u16(nlh, NFTA_LOG_QTHRESHOLD, htons(log->qthreshold));
	if (e->flags & (1 << NFTNL_EXPR_LOG_LEVEL))
		mnl_attr_put_u32(nlh, NFTA_LOG_LEVEL, htonl(log->level));
	if (e->flags & (1 << NFTNL_EXPR_LOG_FLAGS))
		mnl_attr_put_u32(nlh, NFTA_LOG_FLAGS, htonl(log->flags));
}

static int
nftnl_expr_log_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_LOG_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_log_cb, tb) < 0)
		return -1;

	if (tb[NFTA_LOG_PREFIX]) {
		if (log->prefix)
			xfree(log->prefix);

		log->prefix = strdup(mnl_attr_get_str(tb[NFTA_LOG_PREFIX]));
		if (!log->prefix)
			return -1;
		e->flags |= (1 << NFTNL_EXPR_LOG_PREFIX);
	}
	if (tb[NFTA_LOG_GROUP]) {
		log->group = ntohs(mnl_attr_get_u16(tb[NFTA_LOG_GROUP]));
		e->flags |= (1 << NFTNL_EXPR_LOG_GROUP);
	}
	if (tb[NFTA_LOG_SNAPLEN]) {
		log->snaplen = ntohl(mnl_attr_get_u32(tb[NFTA_LOG_SNAPLEN]));
		e->flags |= (1 << NFTNL_EXPR_LOG_SNAPLEN);
	}
	if (tb[NFTA_LOG_QTHRESHOLD]) {
		log->qthreshold = ntohs(mnl_attr_get_u16(tb[NFTA_LOG_QTHRESHOLD]));
		e->flags |= (1 << NFTNL_EXPR_LOG_QTHRESHOLD);
	}
	if (tb[NFTA_LOG_LEVEL]) {
		log->level = ntohl(mnl_attr_get_u32(tb[NFTA_LOG_LEVEL]));
		e->flags |= (1 << NFTNL_EXPR_LOG_LEVEL);
	}
	if (tb[NFTA_LOG_FLAGS]) {
		log->flags = ntohl(mnl_attr_get_u32(tb[NFTA_LOG_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_LOG_FLAGS);
	}

	return 0;
}

static int nftnl_expr_log_json_parse(struct nftnl_expr *e, json_t *root,
					struct nftnl_parse_err *err)
{
#ifdef JSON_PARSING
	const char *prefix;
	uint32_t snaplen, level, flags;
	uint16_t group, qthreshold;

	prefix = nftnl_jansson_parse_str(root, "prefix", err);
	if (prefix != NULL)
		nftnl_expr_set_str(e, NFTNL_EXPR_LOG_PREFIX, prefix);

	if (nftnl_jansson_parse_val(root, "group", NFTNL_TYPE_U16, &group,
				  err) == 0)
		nftnl_expr_set_u16(e, NFTNL_EXPR_LOG_GROUP, group);

	if (nftnl_jansson_parse_val(root, "snaplen", NFTNL_TYPE_U32, &snaplen,
				  err) == 0)
		nftnl_expr_set_u32(e, NFTNL_EXPR_LOG_SNAPLEN, snaplen);

	if (nftnl_jansson_parse_val(root, "qthreshold", NFTNL_TYPE_U16,
				  &qthreshold, err) == 0)
		nftnl_expr_set_u16(e, NFTNL_EXPR_LOG_QTHRESHOLD, qthreshold);

	if (nftnl_jansson_parse_val(root, "level", NFTNL_TYPE_U32, &level,
				  err) == 0)
		nftnl_expr_set_u32(e, NFTNL_EXPR_LOG_LEVEL, level);

	if (nftnl_jansson_parse_val(root, "flags", NFTNL_TYPE_U32, &flags,
				  err) == 0)
		nftnl_expr_set_u32(e, NFTNL_EXPR_LOG_FLAGS, flags);

	return 0;
#else
	errno = EOPNOTSUPP;
	return -1;
#endif
}

static int nftnl_expr_log_snprintf_default(char *buf, size_t size,
					   const struct nftnl_expr *e)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);
	int ret, offset = 0, remain = size;

	if (e->flags & (1 << NFTNL_EXPR_LOG_PREFIX)) {
		ret = snprintf(buf, remain, "prefix %s ", log->prefix);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (e->flags & (1 << NFTNL_EXPR_LOG_GROUP)) {
		ret = snprintf(buf + offset, remain,
			       "group %u snaplen %u qthreshold %u ",
			       log->group, log->snaplen, log->qthreshold);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	} else {
		if (e->flags & (1 << NFTNL_EXPR_LOG_LEVEL)) {
			ret = snprintf(buf + offset, remain, "level %u ",
				       log->level);
			SNPRINTF_BUFFER_SIZE(ret, remain, offset);
		}
		if (e->flags & (1 << NFTNL_EXPR_LOG_FLAGS)) {
			if (log->flags & NF_LOG_TCPSEQ) {
				ret = snprintf(buf + offset, remain, "tcpseq ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
			if (log->flags & NF_LOG_TCPOPT) {
				ret = snprintf(buf + offset, remain, "tcpopt ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
			if (log->flags & NF_LOG_IPOPT) {
				ret = snprintf(buf + offset, remain, "ipopt ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
			if (log->flags & NF_LOG_UID) {
				ret = snprintf(buf + offset, remain, "uid ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
			if (log->flags & NF_LOG_MACDECODE) {
				ret = snprintf(buf + offset, remain,
					       "macdecode ");
				SNPRINTF_BUFFER_SIZE(ret, remain, offset);
			}
		}
	}

	return offset;
}

static int nftnl_expr_log_export(char *buf, size_t size,
				 const struct nftnl_expr *e, int type)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);
	NFTNL_BUF_INIT(b, buf, size);

	if (e->flags & (1 << NFTNL_EXPR_LOG_PREFIX))
		nftnl_buf_str(&b, type, log->prefix, PREFIX);
	if (e->flags & (1 << NFTNL_EXPR_LOG_GROUP))
		nftnl_buf_u32(&b, type, log->group, GROUP);
	if (e->flags & (1 << NFTNL_EXPR_LOG_SNAPLEN))
		nftnl_buf_u32(&b, type, log->snaplen, SNAPLEN);
	if (e->flags & (1 << NFTNL_EXPR_LOG_QTHRESHOLD))
		nftnl_buf_u32(&b, type, log->qthreshold, QTHRESH);
	if (e->flags & (1 << NFTNL_EXPR_LOG_LEVEL))
		nftnl_buf_u32(&b, type, log->level, LEVEL);
	if (e->flags & (1 << NFTNL_EXPR_LOG_FLAGS))
		nftnl_buf_u32(&b, type, log->flags, FLAGS);

	return nftnl_buf_done(&b);
}

static int
nftnl_expr_log_snprintf(char *buf, size_t len, uint32_t type,
			uint32_t flags, const struct nftnl_expr *e)
{
	switch(type) {
	case NFTNL_OUTPUT_DEFAULT:
		return nftnl_expr_log_snprintf_default(buf, len, e);
	case NFTNL_OUTPUT_XML:
	case NFTNL_OUTPUT_JSON:
		return nftnl_expr_log_export(buf, len, e, type);
	default:
		break;
	}
	return -1;
}

static void nftnl_expr_log_free(const struct nftnl_expr *e)
{
	struct nftnl_expr_log *log = nftnl_expr_data(e);

	xfree(log->prefix);
}

static bool nftnl_expr_log_cmp(const struct nftnl_expr *e1,
				     const struct nftnl_expr *e2)
{
	struct nftnl_expr_log *l1 = nftnl_expr_data(e1);
	struct nftnl_expr_log *l2 = nftnl_expr_data(e2);
	bool eq = true;

	if (e1->flags & (1 << NFTNL_EXPR_LOG_SNAPLEN))
		eq &= (l1->snaplen == l2->snaplen);
	if (e1->flags & (1 << NFTNL_EXPR_LOG_GROUP))
		eq &= (l1->group == l2->group);
	if (e1->flags & (1 << NFTNL_EXPR_LOG_QTHRESHOLD))
		eq &= (l1->qthreshold == l2->qthreshold);
	if (e1->flags & (1 << NFTNL_EXPR_LOG_LEVEL))
		eq &= (l1->level == l2->level);
	if (e1->flags & (1 << NFTNL_EXPR_LOG_FLAGS))
		eq &= (l1->flags == l2->flags);
	if (e1->flags & (1 << NFTNL_EXPR_LOG_PREFIX))
		eq &= !strcmp(l1->prefix, l2->prefix);

	return eq;
}

struct expr_ops expr_ops_log = {
	.name		= "log",
	.alloc_len	= sizeof(struct nftnl_expr_log),
	.max_attr	= NFTA_LOG_MAX,
	.free		= nftnl_expr_log_free,
	.cmp		= nftnl_expr_log_cmp,
	.set		= nftnl_expr_log_set,
	.get		= nftnl_expr_log_get,
	.parse		= nftnl_expr_log_parse,
	.build		= nftnl_expr_log_build,
	.snprintf	= nftnl_expr_log_snprintf,
	.json_parse	= nftnl_expr_log_json_parse,
};
