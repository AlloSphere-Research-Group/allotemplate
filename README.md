# allotemplate
Template for creating applications using allolib

Developed by:

AlloSphere Research Group

University of California, Santa Barbara

# Installation

## Dependencies

terminal to run bash

git

cmake version 3.0 or higher

## How to setup
On a bash shell:

    git clone https://github.com/AlloSphere-Research-Group/allotemplate.git <project folder name>
    cd <project folder name>

    ./init.sh

Now the folder is no longer a git repository.

Make the project folder into the git repository of your choice.

Example: Converting the folder into a new github repository (replace username and repository name)

    git init
    git remote add origin git@github.com:username/new_repo
    git push -u origin master

After setting up the git repo, add allolib as a submodule from the root, and perform a recursive update/init

    git submodule add https://github.com/AlloSphere-Research-Group/allolib.git
    git submodule update --recursive --init

## How to compile / run
src folder contains the initial test code you can replace.

Edit CMakeLists.txt and run.sh to match your code.

On a bash shell:

    ./configure.sh

This will execute cmake on the project

    ./run.sh

This will compile the project, and run the binary if compilation is successful.

## How to perform a distclean
If you need to delete the distribution,

    ./distclean.sh

should recursively clean all the build directories of the project including those of allolib and its submodules.
