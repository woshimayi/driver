PROG=	test_url_parse

HTTPOBJS=	extern.o file.o ftp.o http.o progressmeter.o url.o util.o
CFLAGS+=	-I${.CURDIR}/${HTTPREL}
LDADD+=		${HTTPOBJS}

${PROG}: ${HTTPOBJS}

${HTTPOBJS}:
	cd ${.CURDIR}/${HTTPREL} && make $@
	[ -d ${.CURDIR}/${HTTPREL}/obj ] && \
	    ln -sf ${.CURDIR}/${HTTPREL}/obj/$@ . || \
	    ln -sf ${.CURDIR}/${HTTPREL}/$@ .

CLEANFILES=	${HTTPOBJS}

.include <bsd.regress.mk>
