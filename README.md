# Cache_Replacement
## Build the _lightsim_
Type _make_ in sim directory to build _lightsim_. The binary will be in /bin folder


## Trace generation
We use Docker the encapsulate Pin tracer. 
To get a trace, 
-  Build the Docker image using the docker file (after this you need to edit the run_docker.sh so that the Docker image name matches).
- Use the run_docker.sh script to envoke Pin and get trace file.

The format is ./docker_run.sh -o <output_trace_file_name> -t <number_of_trace_to_collect> <the_program_to_run> <the_arguments_of_the_program>

For example running ./docker_run -t 10000 -o ls_trace ls -las

## Memory hierarchy configuration
Editing the memory hierarchy of _lightsim_ is easy. We provide a sample memory hierarchy configuration in the /cfg/cfg.json. 
## Run the simulation
The trace file to be feeded to each CPU can be configed in a JSON file. We provided some simple trace files in /traces folder.

An example to invoke the simulation is provided in /bin/run.sh

./lightsim -c ../cfg/cfg.json -t ../cfg/traces.json -p 1

The command line options are:
```
  -c, --cfg        configuration file in json format (string)
  
  -t, --trace      trace configuration file in json format (string)
  
  -p, --process    processes to simulate (unsigned int)
  
  -n, --inst       simulation instructions (long long [=-1])
  
  -v, --verbose    verbose output
```
