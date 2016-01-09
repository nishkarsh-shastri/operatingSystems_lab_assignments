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

using namespace std;
using namespace htmlcxx;
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

int main()
{
//Parse some html code
  string html = get_file_contents("this.html");
  HTML::ParserDom parser;
  tree<HTML::Node> dom = parser.parseTree(html);
  
  //Print whole DOM tree
  // cout << dom << endl;
  
  //Dump all links in the tree
  tree<HTML::Node>::iterator it = dom.begin();
  tree<HTML::Node>::iterator end = dom.end();
  string base = "http://cse.iitkgp.ac.in/oldlook/";
  string link;
  for (; it != end; ++it)
  {
  	if (it->tagName() == "a" || it->tagName()=="A")
  	{
  		it->parseAttributes();
  		link = it->attribute("href").second;
  		if(link.length()!=0 && link!="#")
  		{
  			std::size_t search = link.find("http");
  			if(search!=std::string::npos && search==0)
  			{
  				cout<<link<<endl;
  			}
  			else
  			{

  				if((search = link.find("mailto:"))==std::string::npos)
  				{
  					link = base+link;
  					cout<<link<<endl;
  				}
  				
  			}

  		}
  	}
  }
}
