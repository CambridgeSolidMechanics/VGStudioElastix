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

#ifdef GNUPLOT
    // Start process for plotting
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    std::string plot_cmd = "gnuplot -p -e \"set key autotitle columnhead; plot for[i = 0:*] file = sprintf('IterationInfo.0.R%i.txt', i) file u 2 w lp title file; while (1) { pause 2; replot }\"";
    CreateProcessA(NULL, (LPSTR)plot_cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
#endif // GNUPLOT

    int retcode = system(cmd.c_str());
    
    fout << "[" << get_time_string() << "]: " << "DVC returned with status " << retcode << std::endl;
    fout.close();
    
#ifdef GNUPLOT
    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#endif // GNUPLOT

    return retcode;
}