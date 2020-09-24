/*
 * (C) 2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2013 by Arturo Borrero Gonzalez <arturo@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/table.h>
#include <libnftnl/common.h>

static struct nftnl_table *table_parse_file(const char *file, uint16_t format)
{
	int fd;
	struct nftnl_table *t;
	struct nftnl_parse_err *err;
	char data[4096];

	t = nftnl_table_alloc();
	if (t == NULL) {
		perror("OOM");
		return NULL;
	}

	fd = open(file, O_RDONLY);
	if (fd < 0) {
		perror("open");
		return NULL;
	}

	if (read(fd, data, sizeof(data)) < 0) {
		perror("read");
		close(fd);
		return NULL;
	}
	close(fd);

	err = nftnl_parse_err_alloc();
	if (err == NULL) {
		perror("error");
		return NULL;
	}

	if (nftnl_table_parse(t, format, data, err) < 0) {
		nftnl_parse_perror("Unable to parse file", err);
		nftnl_parse_err_free(err);
		return NULL;
	}

	nftnl_parse_err_free(err);
	return t;

}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq, table_seq;
	struct nftnl_table *t = NULL;
	int ret, batching;
	uint16_t family, format, outformat;
	struct mnl_nlmsg_batch *batch;

	if (argc < 3) {
		printf("Usage: %s {json} <file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (strcmp(argv[1], "json") == 0) {
		format = NFTNL_PARSE_JSON;
		outformat = NFTNL_OUTPUT_JSON;
	} else {
		printf("Unknown format: only json is supported\n");
		exit(EXIT_FAILURE);
	}

	t = table_parse_file(argv[2], format);
	if (t == NULL)
		exit(EXIT_FAILURE);

	nftnl_table_fprintf(stdout, t, outformat, 0);
	fprintf(stdout, "\n");

	seq = time(NULL);
	batching = nftnl_batch_is_supported();
	if (batching < 0) {
		perror("cannot talk to nfnetlink");
		exit(EXIT_FAILURE);
	}

	batch = mnl_nlmsg_batch_start(buf, sizeof(buf));

	if (batching) {
		nftnl_batch_begin(mnl_nlmsg_batch_current(batch), seq++);
		mnl_nlmsg_batch_next(batch);
	}

	family = nftnl_table_get_u32(t, NFTNL_TABLE_FAMILY);

	table_seq = seq;
	nlh = nftnl_table_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
					NFT_MSG_NEWTABLE, family,
					NLM_F_CREATE|NLM_F_ACK, seq++);
	nftnl_table_nlmsg_build_payload(nlh, t);
	nftnl_table_free(t);
	mnl_nlmsg_batch_next(batch);

	if (batching) {
		nftnl_batch_end(mnl_nlmsg_batch_current(batch), seq++);
		mnl_nlmsg_batch_next(batch);
	}

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, mnl_nlmsg_batch_head(batch),
			      mnl_nlmsg_batch_size(batch)) < 0) {
		perror("mnl_socket_send");
		exit(EXIT_FAILURE);
	}

	mnl_nlmsg_batch_stop(batch);

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, table_seq, portid, NULL, NULL);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		perror("error");
		exit(EXIT_FAILURE);
	}

	mnl_socket_close(nl);

	return EXIT_SUCCESS;
}
