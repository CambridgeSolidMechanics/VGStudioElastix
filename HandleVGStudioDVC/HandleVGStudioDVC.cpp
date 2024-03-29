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

static std::string get_time_string(const char* fmt = "%F %T")
{
    auto time = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(time);
    struct tm localTime;
    localtime_s(&localTime, &timestamp);
    char buf[70];
    strftime(buf, sizeof(buf), fmt, &localTime);
    return std::string(buf);
}

static struct Options
{
    static bool gnuplot, copy_volumes, copy_logs;
    static std::map<std::string, std::string> extra_arguments;
};
bool Options::copy_logs = false;
bool Options::copy_volumes = false;
bool Options::gnuplot = false;
std::map<std::string, std::string> Options::extra_arguments = std::map<std::string, std::string> ();

static void load_options(std::filesystem::path& pth, Options& opt) {
    std::ifstream fin(pth);
    if (!fin.is_open()) {
        std::clog << "Could not open file " << pth.c_str();
        return;
    }
    std::string line;
    while (std::getline(fin, line)) {
        if (line[0] == '-') {
            // take the first word as the key
            string key = line.substr(0, line.find(' '));
            string value = line.substr(line.find(' ') + 1, line.npos);
            opt.extra_arguments[key] = value;
            std::clog << "[" << get_time_string() << "]: " << "Extra argument " << key << ' ' << value << std::endl;
            continue;
        }
        if (line.find("copy_log") != std::string::npos) {
            opt.copy_logs = true;
            std::clog << "[" << get_time_string() << "]: " << "Copying logs" << std::endl;
        }
        else if (line.find("copy_volume") != std::string::npos) {
            opt.copy_volumes = true;
            std::clog << "[" << get_time_string() << "]: " << "Copying volumes" << std::endl;
        } 
        else if (line.find("gnuplot") != std::string::npos) {
            opt.gnuplot = true;
            std::clog << "[" << get_time_string() << "]: " << "Generating gnuplot script" << std::endl;
        }
    }
    fin.close();
    return;
}

static void write_gnuplot_script( std::filesystem::path& folder, bool replot = true ) {
    std::string gp_file = "gplot.gp";
    std::ofstream gp(gp_file, std::ios::out);
    gp << "set key autotitle columnhead" << std::endl;
    gp << "plot for [i = 0:*] ";
    gp << "file = sprintf('" << folder.string() << "\\IterationInfo.0.R%i.txt', i) ";
    gp << "file u 2 w lp title sprintf('R%i',i)" << std::endl;
    if (replot) {
        gp << "while (1) { pause 1; replot }";
    }
    gp.close();
}

int main(int argc, char* argv[])
{
    // Open log file
    std::ofstream fout("DVC_log.txt", std::ios::app);

#ifdef _DEBUG
    auto old_rbuf = std::clog.rdbuf();
    std::clog.set_rdbuf(fout.rdbuf());
#endif // _DEBUG 

    fout << endl;
    fout << "[" << get_time_string() << "]: " << "###### Start DVC ######" << std::endl;

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

    std::string fmod = "DVC_mod_params.txt";
    std::filesystem::path copy_flag_file(fmod);
    Options opt;
    load_options(copy_flag_file, opt);

    // overwrite the arguments from DVC_mod_params.txt
    for (auto it = opt.extra_arguments.begin(); it != opt.extra_arguments.end(); it++) {
        std::string key = it->first;
        std::string value = it->second;
        arguments[key] = value;
    }

    // Modify the parameter file
    std::string params_path = arguments["-p"];
    std::string fout_params = "DVC_run_params.txt";
    int retval = modify_param_files(params_path, fmod, fout_params);
    if (retval < 0) {
        fout << "[" << get_time_string() << "]: " << "Error modifying files" << std::endl;
    }
    arguments["-p"] = fout_params;

    fout << "[" << get_time_string() << "]: " << "Modified parameter file '" << fmod << "' into '" << params_path << "' and saving to '" << fout_params << "'" << std::endl;
    fout.flush();
    
    std::filesystem::path output_path = arguments["-out"];
    if (opt.gnuplot) {
        write_gnuplot_script(output_path);
    }


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

#ifdef _DEBUG
    std::cout.set_rdbuf(old_rbuf);
#endif // _DEBUG

    int retcode = system(cmd.c_str());
    //int retcode = system("timeout /t 5");

    if ( opt.copy_logs || opt.copy_volumes ) {
        std::filesystem::path loc_dir(get_time_string("%Y%m%d_%H%M%S"));
        std::clog << "Creating local directory " << loc_dir.string() << std::endl;
        std::filesystem::create_directory(loc_dir);
        if (opt.copy_logs) {
            std::filesystem::copy(output_path, loc_dir, std::filesystem::copy_options::recursive);
            if (opt.gnuplot) {
                write_gnuplot_script(loc_dir, false);
            }
        }
        if (opt.copy_volumes) {
            std::filesystem::copy(std::filesystem::path(params_path).parent_path(), loc_dir, std::filesystem::copy_options::recursive);
        }
    }
    
    fout << "[" << get_time_string() << "]: " << "DVC returned with status " << retcode << std::endl;
    fout.close();

    return retcode;
}