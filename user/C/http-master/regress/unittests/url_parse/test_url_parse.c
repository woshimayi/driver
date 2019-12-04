/*
 * Copyright (c) 2017 Sunil Nimmagadda <sunil@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <string.h>

#include "ftp.h"

#ifndef nitems
#define nitems(_a)	(sizeof((_a)) / sizeof((_a)[0]))
#endif

static struct {
	const char	*str;
	struct url	 url;
	int		 noparse;
} testcases[] = {
	{ "http://google.com/index.html", {
	    S_HTTP, 0, "google.com", "80", "/index.html" } },
	{ "https://google.com:", {
	    S_HTTPS, 0, "google.com", "443" } },
	{ "file:.", {
	    S_FILE, 0, NULL, NULL, "." } },
	{ "http://[::1]:/index.html", {
	    S_HTTP, 1, "::1", "80", "/index.html" } },
	{ "http://[::1]:1234/", {
	    S_HTTP, 1, "::1", "1234", "/" } },
	{ "foo.bar", {}, 1 },
	{ "http://[::1:1234", {}, 1 },
	{ "http://[1::2::3]:1234", {
	    S_HTTP, 0, "1::2::3", "1234" } },
	{ "http://foo.com:bar", {
	    S_HTTP, 0, "foo.com", "bar" } },
	{ "http:/foo.com", {}, 1 },
	{ "http://foo:bar@baz.com", {
	    S_HTTP, 0, "baz.com", "80" } },
	{ "http://[::1]abcd/", {}, 1 },
	{ "    http://localhost:8080", {
	    S_HTTP, 0, "localhost", "8080" } },
	{ "ftps://localhost:21", {}, 1 },
	{ "http://marc.info/?l=openbsd-tech&m=151790635206581&q=raw", {
	    S_HTTP, 0, "marc.info", "80", "/?l=openbsd-tech&m=151790635206581&q=raw" } },
};

static int
ptr_null_cmp(void *a, void *b)
{
	if ((a && b == NULL) || (a == NULL && b))
		return 1;

	return 0;
}

static int
url_cmp(struct url *a, struct url *b)
{
	if (ptr_null_cmp(a, b) ||
	    ptr_null_cmp(a->host, b->host) ||
	    ptr_null_cmp(a->port, b->port) ||
	    ptr_null_cmp(a->path, b->path))
		return 1;

	if (a->scheme != b->scheme ||
	    (a->host && strcmp(a->host, b->host)) ||
	    (a->port && strcmp(a->port, b->port)) ||
	    (a->path && strcmp(a->path, b->path)))
		return 1;

	return 0;
}

int
main(void)
{
	struct url	*url;
	size_t		 i;

	for (i = 0; i < nitems(testcases); i++) {
		url = url_parse(testcases[i].str);
		if (testcases[i].noparse) {
			if (url != NULL)
				goto bad;
			continue;
		}

		if (url_cmp(url, &testcases[i].url) != 0)
			goto bad;
	}

	return 0;

 bad:
	fprintf(stderr, "%s\n", testcases[i].str);
	return 1;
}
