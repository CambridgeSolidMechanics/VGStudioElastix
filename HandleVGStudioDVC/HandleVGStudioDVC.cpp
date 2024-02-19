// HandleVGStudioDVC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
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

int run_and_log(std::ofstream& fout, std::string cmd) {
    auto time = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(time);
    struct tm localTime;
    localtime_s(&localTime, &timestamp);
    char buf[70];
    strftime(buf, sizeof(buf), "%F %T", &localTime);
    fout << "[" << buf << "]: " << cmd.c_str() << std::endl;
    fout.flush();
    int retcode = system(cmd.c_str());
    return retcode;
}

int main(int argc, char* argv[])
{
    // Dump arguments to file
    std::ofstream fout("DVC_log.txt", std::ios::app);

    // Create command to run
    std::string cmd = "elastix.exe ";
    std::string params_path;

    for (int i = 0; i < argc; i++) {
        //fout << argv[i] << std::endl;
        if (i == 0) {
            continue;
        }
        std::string loc = std::string(argv[i]);
        if (loc.find(' ') != std::string::npos) {
            cmd.append("\"");
            cmd.append(loc);
            cmd.append("\"");
        }
        else {
            cmd.append(loc);
        }
        cmd.append(" ");
        // See if it's parameter file. If so, modify it later
        if (loc.find('-p') != std::string::npos) {
            params_path = std::string(argv[i + 1]);
        }
    }

    std::string sed_cmd = "sed.exe -i 's/AdvancedNormalizedCorrelation/AdvancedMeanSquares/' ";
    sed_cmd.append(params_path);

    auto time = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(time);
    struct tm localTime;
    localtime_s(&localTime, &timestamp);
    char buf[70];
    strftime(buf, sizeof(buf), "%F %T", &localTime);
    fout << "[" << buf << "]: " << sed_cmd.c_str() << std::endl;
    fout.flush();
    int retcode = system(sed_cmd.c_str());


    //int sed_return = run_and_log(fout, sed_cmd);

    //int retcode = run_and_log(fout, cmd);

    time = std::chrono::system_clock::now();
    timestamp = std::chrono::system_clock::to_time_t(time);
    localtime_s(&localTime, &timestamp);
    strftime(buf, sizeof(buf), "%F %T", &localTime);
    fout << "[" << buf << "]: " << cmd.c_str() << std::endl;
    fout.flush();
    retcode = system(cmd.c_str());

    fout.close();

    return retcode;
}