# Chuginate

Chuginate generates a C++ template and build files (makefile + Visual Studio project) for new ChuGins. Use this as a starting point for your new ChuGin!

## Running Chuginate

Before running note the following things:
- As September 2023, chuginate requires Python 3.
- If you are running on Windows you will need some sort of bash environment to run the makefile (WSL, git bash, cygwin, etc.)

(Optional) to re-make your `chuginate` script (one is already provided in this folder):
```
make clean
make
```

To generate a boilerplate project for a new chugin:
1) decide on a name for your new chugin (it's possible to change later, but much easier if you know at this point)
2) create your `destination_directory` to hold the to-be-generated project; e.g., if your new chugin is named "FooBar", you might create `FooBar.chug` or simply `FooBar` as the directory. For this example, we will use the former:
```
mkdir FooBar.chug
```
3) next, run `chuginate [chugin_name] [destination_directory]` to generate a new chugin project in `[destination_directory]`, for example:
```
./chuginate FooBar FooBar.chug
```
(NOTE if this doesn't work, try running the local `chuginate` script: `python ./chuginate [chugin_name] [destination_directory]`.)

4) this should generate a number of boilerplate files in the `[destination_directory`; For example, if "FooBar" is the name of the chugin (and `FooBar.chug` our directory), then in `FooBar.chug/`, we should see the following generated files:
* **FooBar.cpp** the boilerplate C++ source for the chugin (should be buildable without modification; we recommend trying to build and run this before starting to modify)
* **FooBar-test.ck** a boilerplate test ChucK program (try running this after building your chugin; instructions inside)
* **FooBar.vcxproj** Visual Studio project file
* **makefile** makefiles for varius platforms, including `makefile.linux`, `makefile.mac`, and `makefile.win32`
* **chuck/** folder containing chuck header files 

Got questions or want feedback on a chugin you are developing? Join the [ChucK Community Discord server](https://discord.gg/Np5Z7ReesD)!

_Happy chugging!_
