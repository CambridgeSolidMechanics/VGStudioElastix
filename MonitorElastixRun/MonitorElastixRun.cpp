// MonitorElastixRun.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <sstream>
#include <vector>

int main()
{
    std::filesystem::path p = "elastix.log";
	// print full path
	std::cout << "Loading file " << std::filesystem::absolute(p) << std::endl;
    if (std::filesystem::exists(p)) {
		// read the file line by line
		std::ifstream fin(p);
		if (!fin.is_open()) {
			std::cout << "Error opening file" << std::endl;
			return -1;
		}

		std::string line;
		std::vector<std::string> header;
		std::vector<std::vector<float>> data;
		
		bool parse_active = false;
		int num_tokens = 0;

		while (std::getline(fin, line)) {

			if (line.find("Resolution:") != std::string::npos) {
				std::string key = line.substr(line.find(':')+1, line.length());
				int iter_num = std::stoi(key);
				std::cout << "Resolution " << iter_num << std::endl;
			}

			
			if (line.find("1:ItNr") != std::string::npos) {
				num_tokens = 0;
				std::stringstream ss(line);
				std::string token;
				header.clear();
				data.clear();
				while (ss >> token) {
					header.push_back(token);
					data.push_back(std::vector<float>());
					data[data.size() - 1].reserve(2000);
					num_tokens++;
				}
				parse_active = true;
				continue;
			}

			if (parse_active){
				std::stringstream ss(line);
				std::string val_str;
				int counter = 0;
				while (ss >> val_str) {
					if (counter >= num_tokens) {
						std::cout << "breaking" << std::endl;
						break;
					}
					float val;
					try
					{
						val = std::stof(val_str);
					}
					catch (const std::exception&)
					{
						// conversion failed. Presumably we are beyond data
						parse_active = false;
						break;
					}
					data[counter].push_back(val);
					counter++;
				}
				if (!parse_active) {
					// Print out data for each token in header
					for (int i = 0; i < header.size(); i++) {
						std::cout << header[i] << ". Loaded " << data[i].size() << " elements" << std::endl;
					}
				}
			}



		}

		//// Print out data for each token in header
		//for (int i = 0; i < header.size(); i++) {
		//	std::cout << header[i] << ". Loaded " << data[i].size() << " elements" << std::endl;
		//	//for (int j = 0; j < data[i].size(); j++) {
		//	//	std::cout << data[i][j] << ",";
		//	//}
		//	//std::cout << std::endl;
		//}

	}
	else {
		std::cout << "File does not exist" << std::endl;
	}
}
