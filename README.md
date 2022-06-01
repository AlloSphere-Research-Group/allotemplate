# allotemplate
Template for creating applications using Allolib. This template is suitable for large projects wil multiple files and dependencies where you need more control. If you are prototyping single files or want to explore the allolib examples, use the [allolib_playground repo](https://github.com/AlloSphere-Research-Group/allolib_playground).

> Developed by:
>
> AlloSphere Research Group
>
> University of California, Santa Barbara

# Installation
Allotemplate currently requires:
 * bash
 * git
 * cmake version 3.0 or higher

```sh
git clone git@github.com:AlloSphere-Research-Group/allotemplate.git myproject
cd myproject
make run
```

## `alloinit`
Submodule management is handled by the [`alloinit`](utils/alloinit.md) one-step project initializer. You can use it directly to initialize a project:

```sh
# Install `alloinit` to a directory on PATH:
curl -L https://github.com/Allosphere-Research-Group/allotemplate/raw/master/utils/alloinit \
    --output ~/.local/bin/alloinit
export PATH="$HOME/.local/bin:$PATH"
```

```sh
# After installing `alloinit`:
alloinit myproject 
cd myproject
make run
```

`alloinit` offers an alternate project setup mode that shares a single copy of Allolib and its extensions between all projects on a computer that are configured this way. See [its documentation](utils/alloinit.md) for details.

## How to compile and run
The `src/` directory contains source files (`*.cpp`), while the `include/` directory contains header files (`*.hpp`). An example project is included.

To run the project:

```sh
make run
```

To compile the project without running:

```sh
make build
```

To configure CMake without compiling:

```sh
make configure
```

The project is organized into an application half (for executable-specific code) and a library half (for reusable library code). If you would like to rename the two sections (highly recommended):

* Application
  * Replace all instances of `application_name` in `src/app/` with your new application name.
  * Replace the contents of `src/app/name.txt` with your new application name.
* Library
  * Replace all instances of `library_name` in `include/` with your new library name.
  * Rename `include/library_name` to `include/<your new application name>`.
  * Replace the contents of `src/lib/name.txt` with your new library name.

When using the `Makefile` at the project root, CMake will be able to automatically find new or removed files and reconfigure when necessary, so updating `CMakeLists.txt` with manually or with an IDE is not necessary.

## How to clean the repository
If you need to clean up build artifacts, run:

```sh
make clean
```

If you need to completely remove all generated files (including `allolib` and `al_ext`), run:

```sh
make distclean
```

Then, run `make install-deps` to reinstall Allolib and its extensions. The `*-deps` recipes are not compatible with `alloinit`'s shared mode.

## Make recipes
Run `make` or `make help` if you would like to see a summary of available recipes.
