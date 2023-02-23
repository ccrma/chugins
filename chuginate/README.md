# Chuginate

Chuginate generates a C++ template and build files (makefile + Visual Studio project) for new ChuGins. Use this as a starting point for your new ChuGin!

## Running Chuginate

Before running note the following things:

- Make sure Python 2 is installed. If you don't have Python 2 installed already one option is to use 
an environment manager such as conda and add a new environment:

```
conda create -n chugins python=2.7 # makes a new conda environment called chugins with python 2.7
conda activate chugins # switches to the environment
```

- If you are running on Windows you will need some sort of bash environment to run the makefile (WSL, git bash, cygwin, etc.)

To actaully generate your ChuGin template:
```
make clean
make
make install
```

Now, determine your `destination_directory` and make that directory (i.e. `mkdir [destination_director]`).

Next, run `chuginate [chugin_name] [destination_directory]` to create a new chugin from the template. 
If this doesn't work, try running the local `chuginate` script: `python ./chuginate [chugin_name] [destination_directory]`
