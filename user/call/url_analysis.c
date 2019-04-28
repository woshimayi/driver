#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int www_ParseUrl(const char *url, char *proto, char *host, int *port, char *uri)
{
  int n;
  char *p;

  *port = 0;
  strncpy(uri, "",8);

  /* proto */
  p = (char *) url;
  if ((p = strchr(url, ':')) == NULL) {
    return -1;
  }
  n = p - url;
  strncpy(proto, url, n);
  proto[n] = '\0';

  /* skip "://" */
  if (*p++ != ':') return -1;
  if (*p++ != '/') return -1;
  if (*p++ != '/') return -1;

  /* host */
  {
    char *hp = host;
    
    while (*p && *p != ':' && *p != '/') {
      *hp++ = *p++;
    }
    *hp = '\0';
  }
  if (strlen(host) == 0)
    return -1;

  /* end */
  if (*p == '\0') {
    *port = 0;
    strncpy(uri, "",8);
    return 0;
  }

  /* port */
  if (*p == ':') {
    char buf[10];
    char *pp = buf;

    p++;
    while (isdigit(*p)) {
      *pp++ = *p++;
    }
    *pp = '\0';
    if (strlen(buf) == 0)
      return -1;
    *port = atoi(buf);
  }
  
  /* uri */
  if (*p == '/') {
    char *up = uri;
    while ((*up++ = *p++));
  }

  return 0;
}


int main()
{
	char *url = "http://61.163.160.69:8084/webff/versions/3FE56752AOCK07.3FE56752AOCK07";
	char proto[128] = {0};
	char host[128] = {0};
	int port = 0;
	char uri[128] = {0};
	char buf[32] = {0};
	www_ParseUrl(url, proto, host, &port, uri);
	printf("proto = %s host= %s port=%d uri=%s\n", proto, host, port, uri);
	itoa(port, buf, 10);
	printf("%s\n", strcat(host, ":"));
	printf("%s\n", strcat(host, buf));
	return 0;
}


