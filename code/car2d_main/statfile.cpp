#include "statfile.hpp"

StatFile::StatFile(const std::string& filename, const std::string& separator)
	: current_line(0)
	, file(filename, std::ios_base::trunc)
	, separator(separator)
{
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file for writing: " + filename);
	}
}

StatFile::~StatFile()
{
	file.close();
}

void StatFile::commit_line()
{
	// Output headers on the first line.
	if (current_line == 0)
	{
		for (int i = 0; i < column_names.size(); ++i)
		{
			file << column_names[i];
			if (i != column_names.size() - 1)
				file << separator;
		}

		file << std::endl;
	}

	// Output the values.
	for (int i = 0; i < column_names.size(); ++i)
	{
		file << column_values_current[column_names[i]];
		if (i != column_names.size() - 1)
			file << separator;
	}
	
	file << std::endl;

	current_line++;
}