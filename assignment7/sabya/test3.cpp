#include <pthread.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <string>
#include <list>
#include <unistd.h>
# include <string>
# include <iostream>
# include <sstream>
# include <htmlcxx/html/ParserDom.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include <curl/curl.h>
#include <fcntl.h>

#define MAX_LEVEL 	3
#define NTHREADS	5


using namespace std;
using namespace htmlcxx;


typedef struct url_struct {
	std::string url;
	std::string baseurl;
	bool flag;
}url_struct;

struct ele {
	long threadid;
	std::string url;
};



std::queue<url_struct> to_do;
std::queue<url_struct> to_do_next;
std::list<struct ele> done;
int nthreads;
int level;
int number;
pthread_mutex_t mutex;
pthread_cond_t condition;

string to_string(int x)
{
	char str[15];
	sprintf(str, "%d", x);
	string s(str);
	return s;
}
std::string get_file_contents(const char *filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
    std::ostringstream contents;
    contents << in.rdbuf();
    in.close();
    return(contents.str());
  }
  throw(errno);
}

std::list<url_struct> getAllLinks(string filename,url_struct myurl)
{
//Parse some html code
  string html = get_file_contents(filename.c_str());
  HTML::ParserDom parser;
  tree<HTML::Node> dom = parser.parseTree(html);
  
  //Print whole DOM tree
  // cout << dom << endl;
  
  //Dump all links in the tree
  tree<HTML::Node>::iterator it = dom.begin();
  tree<HTML::Node>::iterator end = dom.end();
  string base;
  if(myurl.flag)
    base = myurl.baseurl;
  else
  	base = myurl.url;


  std::list<url_struct> url_list;
  string link;
  url_struct link_struct;
  for (; it != end; ++it)
  {
  	if (it->tagName() == "a")
  	{
  		it->parseAttributes();
  		link = it->attribute("href").second;
  		if(link.length()!=0 && link!="#")
  		{
  			std::size_t search = link.find("http");
  			if(search!=std::string::npos && search==0)
  			{
  				link_struct.url=link;
  				link_struct.baseurl=link;
  				link_struct.flag=0;
  				url_list.push_back(link_struct);
  			}
  			else
  			{

  				if((search = link.find("mailto:"))==std::string::npos)
  				{
  					if (link[0] == '/') 
  						link = base+link;
  					else link = base + "/" + link;
  					link_struct.url = link;
  					link_struct.baseurl = myurl.baseurl;
  					link_struct.flag=1;
  					url_list.push_back(link_struct);  					
  				}
  				
  			}

  		}
  	}
  }
  return url_list;
}

std::list<url_struct> findNeighbours(url_struct myurl,int filenumber)
{
	CURL *curl;
	CURLcode res;
	std::list<url_struct> str_list;
	string url = myurl.url;
	curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
		/* example.com is redirected, so we tell libcurl to follow redirection */ 
		string filename = "urlnumber"+to_string(filenumber);
		filename = "web/"+filename;
		FILE* file = fopen(filename.c_str(), "w");
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK)
		{
		  fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
		  fclose(file);
		  return str_list;
		}	
		fclose(file);
		/* always cleanup */ 
		curl_easy_cleanup(curl);

		str_list = getAllLinks(filename,myurl);
		return str_list;
	}
	return str_list;
}



void *work (void *information) {

	// get thread id
	long threadid = (long)information;

	// lock mutex
	pthread_mutex_lock(&mutex);

	while (level < MAX_LEVEL + 1) {

		bool found = false;
		url_struct url;
		while (to_do.empty() == false && found == false){

			url = to_do.front();
			to_do.pop();
			found = true;
			for (std::list<struct ele>::iterator it = done.begin(); it != done.end(); ++it) {
				if ((*it).url == url.url) {
					found = false;
					break;
				}
			}
		}

		if (found == false) {

			std::cout<<"Thread "<<threadid<<" : found to_do empty"<<std::endl;
			// increment number of threads waiting on empty to_do
			nthreads++;

			if (nthreads == NTHREADS) {
				// swapper thread
				pthread_cond_broadcast(&condition);
				nthreads = 0;
				while (!to_do_next.empty()) {
					url_struct url = to_do_next.front();
					to_do.push(url);
					to_do_next.pop();
				}
				struct ele e;
				e.threadid = -1;
				done.push_back(e);
				std::cout<<std::endl<<"Thread "<<threadid<<" : increasing level "<<level<<" -> "<<level + 1<<std::endl;
				level++;
				pthread_mutex_unlock(&mutex);
			}
			else {
				pthread_cond_wait(&condition, &mutex);
				pthread_mutex_unlock(&mutex);
			}

		} else {

			
			std::cout<<"Thread "<<threadid<<" : dequeuing url "<<url.url<<std::endl;
			struct ele e;
			e.threadid = threadid;
			e.url = url.url;
			done.push_back(e);
			int filenumber = number;
			number++;
			pthread_mutex_unlock(&mutex);

			// insert function here
			std::list<url_struct> neighbours = findNeighbours(url, filenumber);

			pthread_mutex_lock(&mutex);
			for (std::list<url_struct>::iterator it = neighbours.begin(); it != neighbours.end(); it++) {
				to_do_next.push(*it);
			}
			pthread_mutex_unlock(&mutex);

		}
		pthread_mutex_lock(&mutex);
	}

	// unlock mutex
	pthread_mutex_unlock(&mutex);
	// exit thread
	pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
	if (argc != 2) {
		std::cerr<<"usage : url"<<std::endl;
		exit(EXIT_FAILURE);
	}

	// initialize the shared variables
	level = 1;
	nthreads = 0;
	number = 1;
	url_struct myurl;
	myurl.url=argv[1];
	myurl.baseurl=argv[1];
	myurl.flag = 0;
	to_do.push(myurl);

	// initialize the threads, mutex, condition, attributes
	pthread_t threads[NTHREADS];
	pthread_attr_t attr;
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&condition, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// create the threads
	for (long i = 0; i < NTHREADS; ++i) {
		int rv = pthread_create(&threads[i], &attr, work, (void*)(i + 1));
		if (rv != 0) {
			std::cerr<<"Error in creating thread : error code "<<rv<<std::endl;
			exit(EXIT_FAILURE);
		}
	}

	// wait for threads to join
	for (long i = 0; i < NTHREADS; ++i) {
		pthread_join(threads[i], NULL);
	}

	int lvl = 1;
	std::cout<<std::endl<<"depth\tthreadid\turl"<<std::endl;
	for (std::list<struct ele>::iterator it = done.begin(); it != done.end(); ++it) {
		if ((*it).threadid != -1) {
			std::cout<<lvl<<"\t"<<(*it).threadid<<"\t\t"<<(*it).url<<std::endl;
		}
		else {
			std::cout<<std::endl;
			lvl++;
		}
	}

	std::cout<<std::endl;
	lvl++;
	while (!to_do.empty()) {
		url_struct url = to_do.front();
		to_do.pop();
		std::cout<<lvl<<"\t0"<<"\t\t"<<url.url<<std::endl;
	}

	// destroy the attributes, mutex and condition
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&condition);

	// exit main thread
	pthread_exit(NULL);
}