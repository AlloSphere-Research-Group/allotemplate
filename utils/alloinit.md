# `alloinit` ([download][alloinit-download]) #

> One-step [`allotemplate`][allotemplate] initializer.

## Quick Start ##

To start and run a new [`allotemplate`][allotemplate] project:

```sh
curl -L https://github.com/Allosphere-Research-Group/allotemplate/raw/master/utils/alloinit
    | bash -s proj  # Download `alloinit` and initialize an `allotemplate` project in `proj/`.
cd proj             # A copy of `alloinit` is now in `proj/utils`.
./run.sh            # Run your new project!
```

To update [Allolib][allolib] and [its extensions][al_ext]:

```sh
cd proj
utils/alloinit -u .
```

## Description ##

`alloinit` ([download][alloinit-download]) is a one-step initializer for
[`allotemplate`][allotemplate] projects. With one command, `alloinit` can
generate a fully configured project ready to run:

```sh
$ alloinit proj   # Set up `allotemplate` project in `proj/`.
$ cd proj         # Change directories into `proj/`.
$ ./run.sh        # Run the new project!
```

Moreover, `alloinit` can share Allolib and its extensions between projects,
reducing the space utilization on your computer and speeding up the process of
initializing multiple projects on the same machine (so long as they were all
created as shared projects):

```sh
$ # On Computer A:
$ alloinit -S example-project   # Set up a shared project.
$ cd example-project
$ git add . && git commit -m 'Example commit' && git push
$ ./run.sh                      # Ready to run on Computer A!
```

```sh
$ # On Computer B:
$ git clone git@github.com:username/example-project.git
$ cd example
$ alloinit -S -r .              # Link to existing dependencies on Computer B,
                                # or download them if necessary.
$ ./run.sh                      # Ready to run on Computer B!
```

Shared-mode projects are faster to start and clone when using `alloinit`, and
take up less space than normal projects. However, all shared-mode projects on
the same machine will share the same versions of Allolib and extensions—which
may lead to inconsistencies across different machines. Shared mode works by
using symbolic links instead of Git submodules to link `allolib/` and `al_ext/`
to a shared location (defaulting to `$XDG_DATA_HOME/alloinit/` or
`~/.local/share/alloinit/`, but it can be configured by passing `-l <libs>`).
Specifying different directories with the `-l` flag allows you to have multiple
shared dependency folders. `alloinit` comes with a set of utilities for managing
Allolib versions, but because it is all set up with plain symbolic links and
Git, it can all be managed manually if necessary.

The resulting `allotemplate` project will be a Git repository. You can later
pull upstream changes from `allotemplate` using the `allotemplate` remote:

```sh
git pull allotemplate master
```

## Installation ##

`alloinit` does not need to be installed anywhere if you already have an
`allotemplate` project. Simply call `alloinit` from the root of the project (the
one containing the `.git/` directory) using `utils/alloinit` (replace all
instances of `alloinit` in the examples with `utils/alloinit`).

```sh
$ ls -AF  # If you are in the root directory, it should look something like this:
.git/           CMakeLists.txt  al_ext@         bin/            configure.sh*   run.sh*         utils/
.gitignore      README.md       allolib@        build/          debug.sh*       src/
$ utils/alloinit -V
0.2.0
```

If you would like to access `alloinit` from anywhere or from within scripts,
download `alloinit` and move it to a directory on your `PATH`. You can download
`alloinit` from [this link][alloinit-download] or copy it from the `utils/`
subdirectory of an `allotemplate` project. 

### Adding a directory to `PATH` ###

This example creates a directory `~/.local/bin` and adds it to the `PATH`
environment variable. On macOS, replace `~/.profile` with `~/.zprofile`.

```sh
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.profile # ~/.zprofile on macOS
. ~/.profile                                              # ~/.zprofile on macOS
mkdir -p ~/.local/bin/
```

### Installing `alloinit` ###

This example downloads `alloinit` and installs it to `~/.local/bin/`, assuming
that `~/.local/bin/` has been added to the `PATH` environment variable (see
above).

```sh
curl -L https://github.com/Allosphere-Research-Group/allotemplate/raw/master/utils/alloinit \
    > ~/.local/bin/alloinit     # Download `alloinit` to `~/.local/bin/`.
chmod +x ~/.local/bin/alloinit  # Mark `alloinit` as executable.
alloinit -h                     # Run `alloinit`!
```

## Usage ##

### For normal projects ###

Start an [`allotemplate`][allotemplate] project in the directory `proj/`:

```sh
alloinit proj
```

Update Allolib and its extensions to the latest versions:

```sh
alloinit -u proj
```

### For shared-mode projects ###

> Shared-mode projects offer faster setup times and less space usage at the
> expense of control over the specific version of Allolib!

Start a shared-mode [`allotemplate`][allotemplate] project in the directory `proj/`:

```sh
alloinit -S proj
```

Use the directory `libs/` instead of the default shared dependency directory:

```sh
alloinit -S -l libs ...
```

Update the shared Allolib and its extensions to the latest versions:

```sh
alloinit -S -u
```

Set up a shared-mode project `proj` on a new computer, or relink the `allolib`
and `al_ext` symlinks if they were deleted:

```sh
git clone git@github.com:username/proj.git
alloinit -S -r proj
```

Install Allolib and its extensions without creating a new project:

```sh
alloinit -S -L
```

[alloinit-download]: https://github.com/Allosphere-Research-Group/allotemplate/raw/master/utils/alloinit

[allolib]: https://github.com/AlloSphere-Research-Group/allolib
[al_ext]: https://github.com/AlloSphere-Research-Group/al_ext
[allotemplate]: https://github.com/AlloSphere-Research-Group/allotemplate

