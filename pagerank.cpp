#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
using namespace std;


struct Tri_tuple
{
	int row;
	int col;
	double val;
};

bool cmp(pair<int, int> a, pair<int, int> b)
{
	if (a.first < b.first)
	{
		return true;
	}
	else if (a.first > b.first)
	{
		return false;
	}
	else
	{
		return a.second < b.second;
	}
}


int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cout << "usage: ./pagerank [url-file] [result-file]" << endl;
		exit(1);
	}

	map<int, string> url_dict;
	vector<pair<int, int> > link_relation;
	
	ifstream in(argv[1]);
	string str;
	int mode = 0;  // 0 for read url_dict, 1 for read connections
	while (getline(in, str))
    {
        if (str.empty())
        {
            mode = 1;
        }
        else
        {
			if (mode == 0)
			{
				int id;
				string url;
				istringstream ins(str);
				ins >> url >> id;
				url_dict[id] = url;
			}
			else
			{
				int source_id, dest_id;
				istringstream ins(str);
				ins >> source_id >> dest_id;
				link_relation.push_back(pair<int, int>(dest_id, source_id));
			}
        }
    }
	
	in.close();

	// sort the link_relation
	sort(link_relation.begin(), link_relation.end(), cmp);

	// calculate the number of none zero elements in each column
	map<int, int> cols_non_zero_cnt;
	for (auto x : link_relation)
	{
		if (cols_non_zero_cnt.find(x.second) == cols_non_zero_cnt.end())
		{
			cols_non_zero_cnt[x.second] = 1;
		}
		else
		{
			cols_non_zero_cnt[x.second]++;
		}
	}
	
	// construct Gm, Gm doesn't contain any zero elements
	int matrix_size = url_dict.size();
	double alpha = 0.15;
	vector<struct Tri_tuple> Gm;
	for (auto x : link_relation)
	{
		struct Tri_tuple ele;
		ele.row = x.first;
		ele.col = x.second;
		ele.val = 1 / (double)cols_non_zero_cnt[x.second] * (1 - alpha) + alpha * 1 / (double)matrix_size;
		Gm.push_back(ele);
	}

	double vacant_value = alpha * 1 / (double)matrix_size;
	// add last fake element Gm[matrix_size + 1][1] to ensure all_sum[matrix_size] is calculated
	struct Tri_tuple addition;
	addition.row = matrix_size + 1;
	addition.col = 1;
	addition.val = 0;
	Gm.push_back(addition);

	// calculate result
	double *paras = new double[matrix_size + 1];
	double *res = new double[matrix_size + 1];
	double *all_sum = new double[matrix_size + 1];
	double epsilon = 0.0001;
	for (int i = 1; i <= matrix_size; i++)
	{
		paras[i] = 1;
		all_sum[i] = 0;
	}

	while (true)
	{
		// calculate all_sum
		all_sum[0] = 0;
		for (int i = 1; i <= matrix_size; i++)
		{
			all_sum[i] = all_sum[i - 1] + paras[i];
		}

		int prev_row = 0;
		int prev_col = matrix_size;
		double cur_sum = 0;
		for (auto x : Gm)
		{
			int cur_row = x.row;
			int cur_col = x.col;
			double cur_val = x.val;
			// if not dealing with the same row
			if (cur_row != prev_row)
			{
				// calculate the rest sum of the previous row
				cur_sum += (all_sum[matrix_size] - all_sum[prev_col]) * vacant_value;
				res[prev_row] = cur_sum;
				cur_sum = 0;
				prev_col = 0;

				// calculate the result of each row between [prev_row + 1, cur_row - 1]
				for (int row = prev_row + 1; row <= cur_row - 1; row++)
				{
					res[row] = all_sum[matrix_size] * vacant_value;
				}
			}

			// now deal with the current row
			// calculate the sum of cells beteen [prev_col + 1, cur_col - 1]
			if (prev_col + 1 <= cur_col - 1)
			{
				cur_sum += (all_sum[cur_col - 1] - all_sum[prev_col]) * vacant_value;
			}
			//calculate the current value
			cur_sum += cur_val * paras[cur_col];

			// the change the prev_row and prev_col
			prev_row = cur_row;
			prev_col = cur_col;
		}

		// calculate difference
		double diff = 0;
		for (int i = 1; i <= matrix_size; i++)
		{
			diff += (res[i] - paras[i]) * (res[i] - paras[i]);
			// update paras
			paras[i] = res[i];
		}
		if (diff < epsilon * epsilon)
		{
			break;
		}
	}

	// normalize
	double sum_total = 0;
	for (int i = 1; i <= matrix_size; i++)
	{
		sum_total += res[i];
	}
	vector<pair<int, double> > res_inds;
	for (int i = 1; i <= matrix_size; i++)
	{
		res[i] /= sum_total;
		res_inds.push_back(pair<int, double>(i, res[i]));
	}
	sort(res_inds.begin(), res_inds.end(), [](pair<int, double> a, pair<int, double> b) {return a.second > b.second;});
	int max_items = 10;
	ofstream out(argv[2]);
	for (int i = 0; i < max_items; i++)
	{
		out << url_dict[res_inds[i].first] << " " << res_inds[i].second << endl;
	}
	out.close();
	delete [] paras;
	delete [] res;
	delete [] all_sum;
	return 0;
}
