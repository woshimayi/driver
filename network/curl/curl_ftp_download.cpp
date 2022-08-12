/*
 * @*************************************: 
 * @FilePath: \HttpClient-mastery:\Documents\driver\network\curl_ftp_download.c
 * @version: 
 * @Author: dof
 * @Date: 2022-06-17 13:30:35
 * @LastEditors: dof
 * @LastEditTime: 2022-06-17 13:31:51
 * @Descripttion: 
 * @**************************************: 
 */
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h> 
#include "curl/curl.h"
#include <stdio.h>
using namespace std;
 
 
#pragma comment(lib,"libcurl.so")
 
 
struct FtpFile 
{
	const char *filename;
	FILE *stream;
};
 
 
static size_t FetchFiles(void *buffer, size_t size, size_t nmemb, void *stream)
{
	struct FtpFile *out = (struct FtpFile *)stream;
	if (out && !out->stream) 
	{
		// open file for writing 
		out->stream = fopen(out->filename, "wb");
		if (!out->stream)
			return -1; // failure, can't open file to write
	}
	return fwrite(buffer, size, nmemb, out->stream);
}
 
 
int my_progress_func(char *progress_data,  
					 double t, /* dltotal */  
					 double d, /* dlnow */  
					 double ultotal,  
					 double ulnow)  
{  
	printf("%s %g / %g (%g %%)\n", progress_data, d, t, d*100.0/t);  
	return 0;  
}  
 
 
int DownloadFtpFile()
{
	CURL *curl;
	CURLcode res;
	 char *progress_data = "* ";
	struct FtpFile ftpfile = {
		"E:\\123.zzzzz", // name to store the file as if succesful//
		NULL         
	};
    
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
 
	if (curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL," http://down.360safe.com/setup.exe");
		// curl_easy_setopt(curl, CURLOPT_USERPWD, "ljl:521125");
		// Define our callback to get called when there's data to be written //
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FetchFiles);
		// Set a pointer to our struct to pass to the callback //
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);
 
 
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, my_progress_func);  
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data);  
 
		// Switch on full protocol/debug output //
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
 
		res = curl_easy_perform(curl);
 
		// always cleanup 
		curl_easy_cleanup(curl);
 
		if (CURLE_OK != res) 
		{
			//we failed 
			fprintf(stderr, "curl told us %d\n", res);
		}
	}
 
	if (ftpfile.stream)
		fclose(ftpfile.stream); // close the local file 
 
	curl_global_cleanup();
 
	getchar();
 
	return 0;
}
 
 
int main(void)
{
	DownloadFtpFile();
	return 0;
}
