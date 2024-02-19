// ModifyParamFile.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ModifyParamFile.h"

using namespace std;


int main()
{
	modify_param_files("params.txt", "params_mod.txt", "params_torun.txt");
	return 0;
}
