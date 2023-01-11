# allotemplate
Template repository for projects using allolib.
Contains [allolib](https://github.com/AlloSphere-Research-Group/allolib) and [al_ext](https://github.com/AlloSphere-Research-Group/al_ext) as submodules.

This template is suitable for large projects wil multiple files and dependencies where you need more control.

If you are prototyping single files or want to explore the allolib examples, use the [allolib_playground repo](https://github.com/AlloSphere-Research-Group/allolib_playground).

Developed by:
AlloSphere Research Group
University of California, Santa Barbara

# Installation
## Creating a github repository from template
Use https://github.com/AlloSphere-Research-Group/allotemplate/generate

or from https://github.com/AlloSphere-Research-Group/allotemplate,
click on 'Use this template' then 'Create a new repository'.

## Manually creating a new project based on allotemplate
On a bash shell:

    git clone --filter=blob:none --recurse-submodules --also-filter-submodules https://github.com/AlloSphere-Research-Group/allotemplate.git <project folder name>
    cd <project folder name>
    rm -rf .git
    git init
    git remote set-url origin <URL to new repo>

# Building your project
## How to compile / run
The src/ folder contains the initial main.cpp starter code.

On a bash shell you can run:

    ./configure.sh
    ./run.sh

This will configure and compile the project, and run the binary if compilation is successful.

Alternatively, you can open the CMakeLists.txt project in an IDE like VS Code, Visual Studio or Qt Creator and have the IDE manage the configuration and execution of cmake.

You can also generate other IDE projects through cmake.

## How to perform a distclean
If you need to delete the build,

    ./distclean.sh

should recursively clean all the build directories of the project including those of allolib and its submodules.

## Keeping your project up to date
Run

    ./update.sh

or manually run following from a bash shell:

    git pull
    git submodule update --recursive --init --filter=blob:none