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


vector<string> get_page_urls(string page_str, string cur_url)
{
	vector<string> res;
	set<string> all_url;



	// find the structure of all '/'
	vector<int> slash_pos;
	for (int i = 0; i < cur_url.length(); i++)
	{
		if (cur_url[i] == '/')
		{
			slash_pos.push_back(i);
		}
	}

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

				// first the absolute url
				string host = "news.sohu.com";
				size_t found = full_url.find(host);
				if (found != string::npos)
				{
					string url = full_url.substr(found + host.length());

					all_url.insert(cleaned_url(url));
				}
				// second the relative url
				else
				{
					string rel_sibling_prefix = "./";
					string rel_parent_prefix = "../";
					bool is_rel = false;
					int layer = 0;

					while (true)
					{
						if (full_url.substr(0, rel_parent_prefix.length()) == rel_parent_prefix)
						{
							full_url = full_url.substr(rel_parent_prefix.length());
							layer++;
							is_rel = true;
						}
						else if (full_url.substr(0, rel_sibling_prefix.length()) == rel_sibling_prefix)
						{
							full_url = full_url.substr(rel_sibling_prefix.length());
							is_rel = true;
						}
						else
						{
							break;
						}
					}


					if (is_rel)
					{
						int ind = slash_pos.size() - layer - 1;
						if (ind >= 0)
						{
							int pos = slash_pos[ind] + 1;
							string final_url = cur_url.substr(0, pos) + full_url;
							all_url.insert(cleaned_url(final_url));
						}
					}
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

