/*
 * (C) 2012-2014 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>
#include <string.h>
#include <buffer.h>
#include <libnftnl/common.h>
#include <libnftnl/expr.h>
#include "internal.h"

int nftnl_buf_update(struct nftnl_buf *b, int ret)
{
	if (ret < 0) {
		b->fail = true;
	} else {
		b->off += ret;
		if (ret > b->len)
			ret = b->len;
		b->size += ret;
		b->len -= ret;
	}

	return ret;
}

int nftnl_buf_done(struct nftnl_buf *b)
{
	if (b->fail)
		return -1;

	/* Remove trailing comma in json */
	if (b->size > 0 && b->buf[b->size - 1] == ',') {
		b->off--;
		b->size--;
		b->len++;
	}

	return b->off;
}

static int nftnl_buf_put(struct nftnl_buf *b, const char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vsnprintf(b->buf + b->off, b->len, fmt, ap);
	ret = nftnl_buf_update(b, ret);
	va_end(ap);

	return ret;
}

int nftnl_buf_open(struct nftnl_buf *b, int type, const char *tag)
{
	switch (type) {
	case NFTNL_OUTPUT_JSON:
		return nftnl_buf_put(b, "{\"%s\":{", tag);
	case NFTNL_OUTPUT_XML:
	default:
		return 0;
	}
}

int nftnl_buf_close(struct nftnl_buf *b, int type, const char *tag)
{
	switch (type) {
	case NFTNL_OUTPUT_JSON:
		/* Remove trailing comma in json */
		if (b->size > 0 && b->buf[b->size - 1] == ',') {
			b->off--;
			b->size--;
			b->len++;
		}

		return nftnl_buf_put(b, "}}");
	case NFTNL_OUTPUT_XML:
	default:
		return 0;
	}
}

int nftnl_buf_open_array(struct nftnl_buf *b, int type, const char *tag)
{
	switch (type) {
	case NFTNL_OUTPUT_JSON:
		return nftnl_buf_put(b, "{\"%s\":[", tag);
	case NFTNL_OUTPUT_XML:
	default:
		return 0;
	}
}

int nftnl_buf_close_array(struct nftnl_buf *b, int type, const char *tag)
{
	switch (type) {
	case NFTNL_OUTPUT_JSON:
		return nftnl_buf_put(b, "]}");
	case NFTNL_OUTPUT_XML:
	default:
		return 0;
	}
}

int nftnl_buf_u32(struct nftnl_buf *b, int type, uint32_t value, const char *tag)
{
	switch (type) {
	case NFTNL_OUTPUT_JSON:
		return nftnl_buf_put(b, "\"%s\":%u,", tag, value);
	case NFTNL_OUTPUT_XML:
	default:
		return 0;
	}
}

int nftnl_buf_s32(struct nftnl_buf *b, int type, uint32_t value, const char *tag)
{
	switch (type) {
	case NFTNL_OUTPUT_JSON:
		return nftnl_buf_put(b, "\"%s\":%d,", tag, value);
	case NFTNL_OUTPUT_XML:
	default:
		return 0;
	}
}

int nftnl_buf_u64(struct nftnl_buf *b, int type, uint64_t value, const char *tag)
{
	switch (type) {
	case NFTNL_OUTPUT_JSON:
		return nftnl_buf_put(b, "\"%s\":%"PRIu64",", tag, value);
	case NFTNL_OUTPUT_XML:
	default:
		return 0;
	}
}

int nftnl_buf_str(struct nftnl_buf *b, int type, const char *str, const char *tag)
{
	switch (type) {
	case NFTNL_OUTPUT_JSON:
		return nftnl_buf_put(b, "\"%s\":\"%s\",", tag, str);
	case NFTNL_OUTPUT_XML:
	default:
		return 0;
	}
}

int nftnl_buf_reg(struct nftnl_buf *b, int type, union nftnl_data_reg *reg,
		int reg_type, const char *tag)
{
	int ret;

	switch (type) {
	case NFTNL_OUTPUT_XML:
		return 0;
	case NFTNL_OUTPUT_JSON:
		nftnl_buf_put(b, "\"%s\":{", tag);
		ret = nftnl_data_reg_snprintf(b->buf + b->off, b->len, reg,
					    NFTNL_OUTPUT_JSON, 0, reg_type);
		nftnl_buf_update(b, ret);
		return nftnl_buf_put(b, "},");
	}
	return 0;
}

int nftnl_buf_expr_open(struct nftnl_buf *b, int type)
{
	switch (type) {
	case NFTNL_OUTPUT_XML:
		return 0;
	case NFTNL_OUTPUT_JSON:
		return nftnl_buf_put(b, "\"expr\":[");
	}
	return 0;
}

int nftnl_buf_expr_close(struct nftnl_buf *b, int type)
{
	switch (type) {
	case NFTNL_OUTPUT_XML:
		return 0;
	case NFTNL_OUTPUT_JSON:
		nftnl_buf_done(b);
		return nftnl_buf_put(b, "]");
	}
	return 0;
}

int nftnl_buf_expr(struct nftnl_buf *b, int type, uint32_t flags,
		   struct nftnl_expr *expr)
{
	int ret;

	switch (type) {
	case NFTNL_OUTPUT_XML:
		return 0;
	case NFTNL_OUTPUT_JSON:
		nftnl_buf_put(b, "{");
		nftnl_buf_str(b, type, expr->ops->name, TYPE);
		ret = nftnl_expr_snprintf(b->buf + b->off, b->len, expr, type,
					  flags);
		if (ret > 0)
			nftnl_buf_update(b, ret);
		else
			nftnl_buf_done(b);

		return nftnl_buf_put(b, "},");
	}
	return 0;
}
