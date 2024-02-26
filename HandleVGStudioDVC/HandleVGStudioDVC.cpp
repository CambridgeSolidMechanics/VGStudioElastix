// HandleVGStudioDVC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <chrono>
#include <ctime>
#include <time.h>
#include <Windows.h>

#include "ModifyParamFile.h"

#define GNUPLOT

static std::string get_time_string()
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

#ifdef _DEBUG
    auto old_rbuf = std::clog.rdbuf();
    std::clog.set_rdbuf(fout.rdbuf());
#endif // _DEBUG 

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
    int retval = modify_param_files(params_path, fmod, fout_params);
    if (retval < 0) {
        fout << "[" << get_time_string() << "]: " << "Error modifying files" << std::endl;
    }
    arguments["-p"] = fout_params;

    fout << endl;
    fout << "[" << get_time_string() << "]: " << "###### Start DVC ######" << std::endl;
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

#ifdef _DEBUG
    std::cout.set_rdbuf(old_rbuf);
#endif // _DEBUG

    fout.flush();

    std::string gp_file = "gplot.gp";
    std::ofstream gp(gp_file, std::ios::out);
    std::string output_path = arguments["-out"];
    gp << "set key autotitle columnhead" << std::endl;
    gp << "plot for [i = 0:*] ";
    gp << "file = sprintf('" << output_path << "\\IterationInfo.0.R%i.txt', i) ";
    gp << "file u 2 w lp title sprintf('R%i',i)" << std::endl;
    gp << "while (1) { pause 1; replot }";
    gp.close();
    fout << "[" << get_time_string() << "]: " << "Generated gnuplot file" << gp_file << std::endl;


    int retcode = system(cmd.c_str());
    //int retcode = system("timeout /t 5");

    
    fout << "[" << get_time_string() << "]: " << "DVC returned with status " << retcode << std::endl;
    fout.close();

    return retcode;
}