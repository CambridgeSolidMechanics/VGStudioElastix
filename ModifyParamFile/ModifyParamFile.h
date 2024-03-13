#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <filesystem>

using namespace std;

int modify_param_files(const string& fin, const string& fmod, const string& fout)
{
	// Open the input and mod files for reading and the output file for writing
	filesystem::path cdir = filesystem::current_path();
	clog << "Current folder " << cdir.string() << endl;
	clog << "Merging parameters from '" << fmod << "' into '" << fin << "' and output to '" << fout << "'" << endl;
	ifstream finfile(cdir / fin, ios::in);
	ifstream fmodfile(cdir / fmod, ios::in);
	ofstream foutfile(cdir / fout, ios::out);

	string line, lstrip;
	map<string, string> params_enforce;

	// Read through fmod and put lines into a map with the parameter name as the key
	if (!fmodfile.is_open()) {
		clog << "File '" << (cdir / fmod).string() << "' exists? " << filesystem::exists(cdir / fmod) << endl;
		clog << "File '" << fmod << "' not open" << endl;
		clog << "Create file '" << fmod << "'" << endl;
		ofstream f_loc_out(cdir / fmod, ios::out);
		f_loc_out.close();
	}
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
	clog << "Read " << params_enforce.size() << " elastix parameter lines from file '" << fmod << "'" << endl;

	// Read through fin. If the line is in the map, replace it with the value from the map
	// Otherwise, leave the same. Either way write the line to fout
	if (!finfile.is_open()) {
		clog << "File '" << fin << "' not open" << endl;
		return -1;
	}
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

	return 0;
}