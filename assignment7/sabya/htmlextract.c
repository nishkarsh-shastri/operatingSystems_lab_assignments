#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <unistd.h>
 
int main(int argc, const char *argv[])
{
  if (argc != 3) {
    printf("usage : url output_file (example : http://cse.iitkgp.ac.in out.html)\n");
    exit(1);
  }
  CURL *curl;
  CURLcode res;
 
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
    /* example.com is redirected, so we tell libcurl to follow redirection */ 
    FILE* file = fopen(argv[2], "w");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
    fclose(file);
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  return 0;
}


std::list<std::string> findNeighbours(std::string baseurl)
{
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if(curl) {
	curl_easy_setopt(curl, CURLOPT_URL, argv[1]);
	/* example.com is redirected, so we tell libcurl to follow redirection */ 
	FILE* file = fopen(argv[2], "w");
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

	/* Perform the request, res will get the return code */ 
	res = curl_easy_perform(curl);
	/* Check for errors */ 
	if(res != CURLE_OK)
	  fprintf(stderr, "curl_easy_perform() failed: %s\n",
	          curl_easy_strerror(res));
	fclose(file);
	/* always cleanup */ 
	curl_easy_cleanup(curl);
	}	
}