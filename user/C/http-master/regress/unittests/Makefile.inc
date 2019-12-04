REGRESS_FAIL_EARLY=	Yes
WARNINGS=		Yes
DEBUG=			-g

HTTPREL=	../../..
LDADD+=		-lutil -ltls -lssl -lcrypto
DPADD+=		${LIBUTIL} ${LIBTLS} ${LIBSSL} ${LIBCRYPTO}
