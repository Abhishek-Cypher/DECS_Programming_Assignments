#include "http_server.hh"

#include <vector>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
 
vector<string> split(const string &s, char delim)
{
  vector<string> elems;

  stringstream ss(s);
  string item;

  while (getline(ss, item, delim))
  {
    if (!item.empty())
      elems.push_back(item);
  }

  return elems;
}

HTTP_Request::HTTP_Request(string request)
{
  vector<string> lines = split(request, '\n');
  vector<string> first_line = split(lines[0], ' ');

  this->HTTP_version = "1.0"; // We'll be using 1.0 irrespective of the request

  /*
   TODO : extract the request method and URL from first_line here
  */
  this->method = first_line[0];
  this->url = first_line[1];

  if (this->method != "GET")
  {
    cerr << "Method '" << this->method << "' not supported" << endl;
    exit(1);
  }
}

HTTP_Response *handle_request(string req)
{
  HTTP_Request *request = new HTTP_Request(req);

  HTTP_Response *response = new HTTP_Response();

  string url = string("html_files") + request->url;

  response->HTTP_version = "1.0";

  // MY DATE CODE ---------------------------------------------------
  time_t curtime = time(NULL);
  struct tm *restime = localtime(&curtime);
  response->date = asctime(restime);
  // MY DATE CODE ---------------------------------------------------
	
  struct stat sb;
  if (stat(url.c_str(), &sb) == 0) // requested path exists
  {
    response->status_code = "200";
    response->status_text = "OK";
    response->content_type = "text/html";

    if (S_ISDIR(sb.st_mode))
    {
      /*
      In this case, requested path is a directory.
      TODO : find the index.html file in that directory (modify the url
      accordingly)
      */
      string file_loc;
      file_loc = url + "/index.html";
      url = file_loc;
    }

    /*
    TODO : open the file and read its contents
    */
    fstream fs;
    string openbuf;
    string file_body;

    fs.open(url);
    if (fs)
    {
      while (getline(fs, openbuf))
      {
        file_body += openbuf;
        file_body += "\n";
      }
    }
    else // IF INDEX.HTML DOES NOT EXISTS, fs RETURNS NON INT
    {
      response->status_code = "404";
      response->status_text = "Not Found";
      file_body = "THE GIVEN DIRECTORY DOES NOT CONTAIN index.html";
    }

    response->body = file_body;
    response->content_length = to_string(file_body.size());
	
    fs.close();	
    /*
    TODO : set the remaining fields of response appropriately
    */
    
    delete request;

    return response;
  }

  else
  {
    response->status_code = "404";
    response->status_text = "Not Found";
    response->content_type = "text/html";
    response->body = "<h1> ERROR 404 : PAGE NOT FOUND </h1>";
    response->content_length = to_string(response->body.length());

    /*
    TODO : set the remaining fields of response appropriately
    */
  }

  delete request;

  return response;
}

string HTTP_Response::get_string()
{
  /*
  TODO : implement this function
  */
  string resp = "";
  resp += string("HTTP/") + this->HTTP_version + string(" ") + this->status_code + string(" ") + this->status_text + string("\r\n");
  resp += string("Date: ") + this->date;
  resp += string("Content-Length: ") + this->content_length + string("\r\n");
  resp += string("Content-Type: ") + this->content_type + string("\r\n\r\n");
  resp += this->body;

  // cout << resp;
  return resp;
}
