#pragma once

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>

class StatFile
{
public:
	StatFile(const std::string& filename, const std::string& separator = ",");
	~StatFile();

	template <typename T>
	void add_column(const std::string& column_name, const T& value);
	
	void commit_line();
private:
	int current_line;
	std::vector<std::string> column_names;
	std::map<std::string, std::string> column_values_current;
	std::ofstream file;
	std::string separator;
};

template <typename T>
void StatFile::add_column(const std::string& column_name, const T& value)
{
	bool column_exists = false;
	if (std::find(column_names.begin(), column_names.end(), column_name) != column_names.end())
	{
		column_exists = true;
	}
	else
	{
		if (current_line == 0)
		{
			// New lines can only be added the first time around.
			column_names.push_back(column_name);
			column_exists = true;
		}
	}

	if (column_exists)
	{
		std::stringstream ss;
		ss << value;
		column_values_current[column_name] = ss.str();
	}
}