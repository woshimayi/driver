/*
 * @*************************************:
 * @FilePath: /network/curl/fileupload.c
 * @version:
 * @Author: dof
 * @Date: 2023-09-11 11:50:24
 * @LastEditors: dof
 * @LastEditTime: 2023-09-13 14:09:14
 * @Descripttion:
 * @**************************************:
 */

#include <stdio.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
  CURL *curl;
  CURLcode res;
  struct stat file_info;
  double speed_upload, total_time;
  FILE *fd;

  struct curl_httppost *post = NULL;
  struct curl_httppost *last = NULL;

  fd = fopen("http2-download.c", "rb"); /* open file to upload */
  if (!fd)
  {
    return 1; /* can't continue */
  }

  /* to get the file size */
  if (fstat(fileno(fd), &file_info) != 0)
  {
    return 1; /* can't continue */
  }

  curl = curl_easy_init();
  if (curl)
  {
    /* upload to this place */
    // curl_easy_setopt(curl, CURLOPT_URL,
    //                  "http://172.16.28.157:8056/upload");

    curl_easy_setopt(curl, CURLOPT_URL,
                     "http://172.16.28.157:8080/u");

    curl_formadd(&post, &last, CURLFORM_PTRNAME, "file", CURLFORM_FILE, "./", CURLFORM_FILENAME, "new", CURLFORM_END);
    /* tell it to "upload" to the URL */
    // curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    /* set where to read from (on Windows you need to use READFUNCTION too) */
    curl_easy_setopt(curl, CURLOPT_READDATA, fd);

    /* and give the size of the upload (optional) */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                     (curl_off_t)file_info.st_size);

    /* enable verbose for easier tracing */
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    }
    else
    {
      /* now extract transfer info */
      curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
      curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);

      fprintf(stderr, "Speed: %.3f bytes/sec during %.3f seconds\n",
              speed_upload, total_time);
    }
    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  return 0;
}
