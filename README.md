# allotemplate
Template for creating applications using allolib. This template is suitable for large projects wil multiple files and dependencies where you need more control. If you are prototyping single files or want to explore the allolib examples, use the [allolib_playground repo](https://github.com/AlloSphere-Research-Group/allolib_playground).

Developed by:

AlloSphere Research Group

University of California, Santa Barbara

# Installation

Allotemplate currently requires:
 * bash shell
 * git
 * cmake version 3.0 or higher

## Creating a new project based on allotemplate
On a bash shell:

    git clone https://github.com/AlloSphere-Research-Group/allotemplate.git <project folder name>
    cd <project folder name>
    ./init.sh

This will prepare the project as a fresh git repository and will add allolib and al_ext as submodules.

## How to compile / run
The src/ folder contains the initial main.cpp starter code.

On a bash shell you can run:

    ./configure.sh
    ./run.sh

This will configure and compile the project, and run the binary if compilation is successful.

Alternatively, you can open the CMakeLists.txt proeject in an IDE like VS Code, Visual Studio or Qt Creator and have the IDE manage the configuration and execution of cmake.

You can also generate other IDE projects through cmake.

## How to perform a distclean
If you need to delete the build,

    ./distclean.sh

should recursively clean all the build directories of the project including those of allolib and its submodules.
