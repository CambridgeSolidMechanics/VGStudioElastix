// HandleVGStudioDVC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <chrono>
#include <ctime>
#include <time.h>

#include "ModifyParamFile.h"

std::string get_time_string()
{
	auto time = std::chrono::system_clock::now();
	std::time_t timestamp = std::chrono::system_clock::to_time_t(time);
	struct tm localTime;
	localtime_s(&localTime, &timestamp);
	char buf[70];
	strftime(buf, sizeof(buf), "%F %T", &localTime);
	return std::string(buf);
}

int main(int argc, char* argv[])
{
    // Dump arguments to file
    std::ofstream fout("DVC_log.txt", std::ios::app);

    // Create command to run
    std::string cmd = "elastix.exe ";
    std::map<std::string, std::string> arguments;

    for (int i = 0; i < argc; i++) {
        if (i == 0) {
            continue;
        }
        if (argv[i][0] == '-') { // parameter
            arguments[argv[i]] = argv[i + 1];
            i++; // skip the next argument
        }
    }

    // Modify the parameter file
    std::string params_path = arguments["-p"];
    std::string fmod = "DVC_mod_params.txt";
    std::string fout_params = "DVC_run_params.txt";
    modify_param_files(params_path, fmod, fout_params);
    arguments["-p"] = fout_params;

    fout << "[" << get_time_string() << "]: " << "Modified parameter file '" << fmod << "' into '" << params_path << "' and saving to '" << fout_params << "'" << std::endl;
    fout.flush();

    // Compose and run the command
    for (auto it = arguments.begin(); it != arguments.end(); it++) {
        std::string value = it->second;
        if (value.find(" ") != std::string::npos) {
			value = "\"" + value + "\"";
		}
		cmd += it->first + " " + value + " ";
	}
    fout << "[" << get_time_string() << "]: " << cmd.c_str() << std::endl;
    fout.flush();
    fout.close();

    int retcode = system(cmd.c_str());

    return retcode;
}