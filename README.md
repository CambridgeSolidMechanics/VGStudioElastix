# VGStudioElastix
## Background
VG Studio runs DVC analysis by executing `dvc.exe` executable from the installation directory with some input arguments.
The call looks like:

```
C:/Program Files/Volume Graphics/VGSTUDIO MAX 20233/dvc.exe -out [output_directory] -p [params_file] -f [fixed_volume] -m [moving_volume]
```

We can create a wrapper around the executable and name it `dvc.exe` to force VG Studio to run with our parameters. 
The call from our wrapper is something like:
```
elastix.exe -out [output_directory] -p [modified_params_file] -f [fixed_volume] -m [moving_volume]
```
where the executable `elastix.exe` is compiled _elastix_<sup>[1](#myfootnote1)</sup> package (we use the lastest version of the package).

## Usage
1. Open the project in Visual Studio. Choose the 'HandleVGStudioDVC' as the startup project. Compile the Visual Studio project. It should be straightforward as it only has dependencies from the C++ standard library.
2. Take the resulting executable `HandleVGStudioDVC.exe`, rename it to `dvc.exe` and paste it in the VG Studio installation directory. (Create a backup copy of the original `dvc.exe` file and replace it with this version.)
3. Take elastix <sup>[1](#myfootnote1)</sup> binary (from official release, or compile it yourself) and place it in the same directory as the VG Studio '.vgl' file which will be running the analysis. You'll also need to copy the associated dll `ANNlib-5.1.dll`
4. Run DVC analysis from VG Studio as before. New files will be created in the directory: `DVC_mod_params.txt`, `DVC_log.txt` and `DVC_run_params.txt`. Their purpose is as follows:
   (i) `DVC_log.txt` is a log file. Use for troubleshooting. (ii) `DVC_mod_params.txt` will be initially empty. This file is the one we can modify to overwrite the default set of parameters.
   (iii) `DVC_run_params.txt` is the file with the final set of parameters which will be used for the analysis.
5. Modify the `DVC_mod_params.txt` file if needed. Any line from `DVC_run_params.txt` can be copied and changed. Running the analysis from VG Studio again will result in an analysis with the modified option.
   Some other settings that we implemented are:
   - including line `// copy_log` or `// copy_logs` will copy text logs from the temporary output directory into a subfolder in the current directory
   - including line `// copy_volume` or `// copy_volumes` will copy the volume files from the temporary output directory into a subfolder in the current directory
   - including line `// gnuplot` will produce a gnuplot script linked to the (temporary) output directory. The script can be used to monitor convergence during the analysis. If `// copy_logs` is also set, at the end of the run the gnuplot script will change to point to the subfolder with copied logs.
6. Run VG Studio DVC analysis again to take the modified parameters into effect. Results will be read and visualized by VG Studio as before.

## Referenes
<a name="myfootnote1">1</a>: https://github.com/SuperElastix/elastix
