# al_proj

initialize repo with

	git submodule init
	git submodule update


LINUX/MACOS:

	./run.sh ${folder_of_the_project}

add -d to debug

	run.sh -d ${folder_of_the_project}
	

WINDOWS

generate VS2015 projects

- run cmake for al_lib and build the library,

- run cmake for project folder and work in visual studio solution.