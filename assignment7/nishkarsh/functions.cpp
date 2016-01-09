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

typedef struct url_struct
{
	std::string url;
	std::string baseurl;
	bool flag;
}url_struct;	

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
    base = myurl.baseurl+"/";
  else
  	base = myurl.url+"/";


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
  					link = base+link;
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
	cout<<"Finding files hellp"<<endl;
	string url = myurl.url;
	curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL,url.c_str());
		/* example.com is redirected, so we tell libcurl to follow redirection */ 
		string filename = "urlnumber"+to_string(filenumber);
		cout<<"filename is "<<filename<<endl;
		cout<<"Curling "<<url<<endl;

		FILE* file = fopen(filename.c_str(), "w");
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

		/* Perform the request, res will get the return code */ 
		res = curl_easy_perform(curl);
		printf("Curling\n");
		/* Check for errors */ 
		if(res != CURLE_OK)
		{
		  fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
		  fclose(file);
		  return str_list;
		}	
		fclose(file);
		cout<<"Finding files"<<endl;
		/* always cleanup */ 
		curl_easy_cleanup(curl);

		str_list = getAllLinks(filename,myurl);
		for(list<url_struct>::iterator it=str_list.begin();it!=str_list.end();it++)
		{
			cout<<"MAIN URL :: "<<(*it).url<<endl;
			cout<<"Base URL :: "<<(*it).baseurl<<endl;
			cout<<"Flag  :: "<<(*it).flag<<endl;
		}
		return str_list;
	}
	return str_list;
}

int main()
{
	std::list<url_struct> str_list;
	url_struct myurl;
	myurl.url="http://cse.iitkgp.ac.in";
	myurl.baseurl="http://cse.iitkgp.ac.in";
	myurl.flag = 0;
	int i = 1;
	str_list=findNeighbours(myurl,1);
	for(list<url_struct>::iterator it=str_list.begin();it!=str_list.end();it++)
	{
		i++;
		findNeighbours(*it,i);
	}
}