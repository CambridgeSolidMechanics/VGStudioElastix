// ModifyParamFile.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <map>

#include "ModifyParamFile.h"

using namespace std;

void modify_param_files(string fin, string fmod, string fout)
{
	// Open the input and mod files for reading and the output file for writing
	cout << "Merging parameters from '" << fmod << "' into '" << fin << "' and output to '" << fout << "'" << endl;
	ifstream finfile(fin);
	ifstream fmodfile(fmod);
	ofstream foutfile(fout, ios::out);

	string line, lstrip;
	map<string, string> params_enforce;

	// Read through fmod and put lines into a map with the parameter name as the key
	while (getline(fmodfile, line))
	{
		if (line[0] == '(' && line[line.size() - 1] == ')')
		{
			// strip brackets and take the first word as the key
			lstrip = line.substr(1, line.size() - 2);
			string key = lstrip.substr(0, lstrip.find(' '));
			params_enforce[key] = line;
		}
	}

	// Read through fin. If the line is in the map, replace it with the value from the map
	// Otherwise, leave the same. Either way write the line to fout
	while (getline(finfile, line))
	{
		// if string starts and ends with a bracket
		if (line[0] == '(' && line[line.size() - 1] == ')')
		{
			// strip brackets and take the first word as the key
			lstrip = line.substr(1, line.size() - 2);
			string key = lstrip.substr(0, lstrip.find(' '));
			if (params_enforce.find(key) != params_enforce.end())
			{
				foutfile << params_enforce[key] << endl;
				params_enforce.erase(key);
			}
			else
			{
				foutfile << line << endl;
			}
		}
		else
		{
			foutfile << line << endl;
		}
	}
	// Write any remaining lines from fmod to fout
	for (auto it = params_enforce.begin(); it != params_enforce.end(); it++)
	{
		foutfile << it->second << endl;
	}

	finfile.close();
	fmodfile.close();
	foutfile.close();
}

int main()
{
	modify_param_files("params.txt", "params_mod.txt", "params_torun.txt");
	return 0;
}
