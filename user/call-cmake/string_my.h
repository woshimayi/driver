




char *strrpl(char * s, const char * s1, const char * s2);




char *strrpl(char * s, const char * s1, const char * s2)
{
	char * ptr;
	
	while(ptr =  strstr(s, s1))
	{
		memmove(ptr+strlen(s2), ptr+strlen(s1), strlen(ptr)-strlen(s1)+1);
		memcpy(ptr, &s2[0], strlen(s2));
	}
}