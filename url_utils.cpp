#include <set>

#include "url_utils.h"


string cleaned_url(string url)
{
	// eliminate '\n' '\t' ' '
	string eliminate_strs[] = {"\n", "\t", " ", "#"};
	for (auto x:eliminate_strs)
	{
		size_t pos = url.find(x);
		if (pos != string::npos)
		{
			url = url.substr(0, pos);	
		}
	}
	return url;
}


vector<string> get_page_urls(string page_str)
{
	vector<string> res;
	set<string> all_url;

	int state = 0;
	int start_ind = 0;
	int end_ind = 0;
	
	for (int i = 0; i < page_str.length(); i++)
	{
		char cur_ch = page_str[i];
		if (state == 0)
		{
			if (cur_ch == '<')
			{
				state = 1;
			}
		}
		else if (state == 1)
		{
			if (cur_ch == 'a')
			{
				state = 2;
			}
			else
			{
				state = 0;
			}
		}
		else if (state == 2)
		{
			if (cur_ch == 'h')
			{
				state = 3;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
		}
		else if (state == 3)
		{
			if (cur_ch == 'r')
			{
				state = 4;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 4)
		{
			if (cur_ch == 'e')
			{
				state = 5;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 5)
		{
			if (cur_ch == 'f')
			{
				state = 6;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 6)
		{
			if (cur_ch == '=')
			{
				state = 7;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 7)
		{
			if (cur_ch == '"')
			{
				// start record website
				start_ind = i;
				state = 8;
			}
			else if (cur_ch == '>')
			{
				state = 0;
			}
			else
			{
				state = 2;
			}
		}
		else if (state == 8)
		{
			if (cur_ch == '"')
			{
				// end record website
				end_ind = i;
				string full_url = page_str.substr(start_ind + 1, end_ind - start_ind - 1);
				string host = "news.sohu.com";
				size_t found = full_url.find(host);
				if (found != string::npos)
				{
					string url = full_url.substr(found + host.length());

					all_url.insert(cleaned_url(url));
				}
				state = 0;
			}
		}
	}

	for (auto url:all_url)
	{
		res.push_back(url);
	}
	return res;
}

