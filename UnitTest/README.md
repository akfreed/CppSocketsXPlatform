# Development Guide

## TL;DR

Use CMake for build and tests. Be sure to initialize the submodules.

Linting is done automatically on GitHub PR. If you want to do it manually, install the linting tools and use pre-commit.

* Ubuntu
  * install [Homebrew](https://brew.sh/)
  * `brew install llvm cppcheck`
  * `pip install pre-commit cpplint clang-format`
* Windows
  * Install [Chocolatey](https://chocolatey.org/)
  * `choco install llvm cppcheck ninja`
  * `pip install pre-commit cpplint`
  * Create a build folder for Ninja and run `cmake .. -G Ninja` in an admin console to generate a `compile_commands.json`

`pre-commit run --all-files`

## Environment

Windows and Linux are supported.

### Required tools for building

* OS
  * Ubuntu 18+
  * Windows 7+
* Compilers
  * GCC
  * Visual Studio 2019+
    * MSVC 16.10+ strongly recommended
* CMake 3.15+
  * 3.21+ strongly recommended if using Visual Studio

### Required tools for testing

None.

### Required tools for linting

* Python 3.6+
* Cppcheck 2.7+
* cpplint 1.6+
* clang-format 14+
* clang-tidy 13+

Additional for Linux

* Homebrew is strongly recommended.

Additional for Windows

* Ninja

## Build

Clone the repo. Checkout the correct branch. Initialize the submodules with `git submodule --init --recursive` . This order is required for now because the submodules aren't on the master branch yet. After that, you can clone with the `--recursive` flag instead of manually initializing the submodules.

Create a build directory under the source root directory. Then follow the specific steps for you system.

### Linux

```
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j
```

### Windows

```
cmake ..
```

Open the generated solution and use the IDE to build.

## Tests

### C++ unit tests

The test library used is GoogleTest. It is automatically downloaded as a submodule if you didn't forget to set that up after you cloned this repo.

To run the unit tests, build and execute the `UnitTest` binary/project.

### Python unit tests

There is also a single python test under scripts for testing `scripts/pocc-shim.py`. It only needs to be run if you change `pocc-shim.py`. 

You will have to install `pytest` via `pip`. See the instructions under the Linting section for how to setup Python and pip. Then do `pip install pytest`.

Then `cd` to `scripts/` and run `pytest`.

## Linting

In this project, the GitHub Workflow CI is configured to run linting automatically. It is not required to setup on your local dev environment, but it is recommended. You may find it useful and it might save you time to in the long-run if you're troubleshooting a particular issue or trying to iterate in defiance of clang-format.

This project is configured for pre-commit to do linting. This mean you will need Python as well as the linting programs if you setup locally.

### Tools

**Cppcheck** is a static analysis tool developed by Daniel Marjamäki and written in C++. It doesn't have too many checks, but the checks it does are strong and it doesn't give many false positives.

**Cpplint** is a static analysis and style-checking tool for C++ developed by Google and written in Python. It has no options for using a different style.

**clang-format** is a customizable auto-formatter developed by LLVM and written in C++. I like it because the customizations are relatively quick and easy to setup. I was able to set it pretty close to my desired style, although I needed to compromise. **Be careful when running pre-commit manually, as it will unforgivingly auto-edit your unstaged files.**

**clang-tidy** is a static analysis and style-checking tool developed by LLVM and written in C++. It does many different kinds of checks for things like bugs, security, performance, style, etc. It's the Sterling Archer of linters. It's a pain to setup and work with. It takes a long time and uses all your resources. It does a lot and it's really good at its job, but it gets distracted easily and makes so much noise that you'll end up ignoring everything it says. 

It is critical to have at least the minimum versions of the linting tools. Cross-platform C++ linting tools in general are surprisingly primitive. Lots of brilliant people and huge companies work on them. The language itself is well-studied and has been around longer than half the people who use it. Yet, modern-day C++ linting tools are either very rudimentary or very difficult to setup and give too many false errors. It's important to have the latest versions to get better checks and better control over the flow. This is particularly an issue for Linux distributions such as Ubuntu and CentOS, which run stable versions of packages on long-term-support releases.

This is why Homebrew is recommended for Linux. Both Homebrew and Chocolatey for Windows have recent versions of various packages.

Getting the correct version is easier on Windows since you tend to be able to download an installer for a particular version.

### Package manager

As discussed, you *must* have at least the minimum versions (listed earlier) of the linting tools. 

For Windows, the package-manager Chocolatey is strongly recommended. It is incredibly easy to install, only requiring you to run a one-liner in an administrative PowerShell prompt. Go to their website to get the one-liner. https://chocolatey.org/install

For Linux, the package-manager Homebrew is strongly recommended. It is incredibly easy to install, only requiring a little copy-paste. You will put run a one-liner, then **READ the output** and follow the instructions at the end, which will involve a copy-paste. Go to their website to get the one-liner. https://brew.sh/

If you are on Linux and don't want to install Homebrew (or some other package manager that hosts the latest versions of packages), there are two alternatives. You can try to find and download a pre-built binary or package online (not recommended). Or you can clone the tool's repo and build it from source.

Building from source is usually surprisingly easy, since Linux people are all about that and Linux projects are generally setup to streamline this. But for building a project, it's not always straightforward knowing how to get started.

### Python

For Windows, go to the Python website to download an installer or use Chocolatey.

For Ubuntu, use:

 `sudo apt update && sudo apt install python-is-python3 python3-pip python3-venv`.

Now that you have Python installed, create a virtual environment in this project's source tree like this:

`python -m venv .venv`

This installs a local copy of Python in a folder named `.venv`.

Then whenever you open a new terminal, activate it like this:

Linux: `. .venv/bin/activate`

Windows: `.venv\Scripts\activate`

This sets up the environment variables to call the python, pip, and other packages installed locally in that virtual environment. New packages you install with pip will be installed here while the environment is active. You can deactivate the environment by entering `deactivate`. If you are done with an environment forever, just delete the `.venv` folder.

`pip` is Python's package manager. First it needs to be upgraded.

Linux: `pip install -U pip`

Windows: The Linux command should work now. Older versions of Python you had to use `easy_install -U pip`

### Ninja

Clang-tidy needs a `compile_commands.json` file from the build system. On Linux this is generated by Make during CMake config, so nothing extra is required.

For Windows, you will need Ninja.  MSVC does not generate a `compile_commands.json`, but Ninja does. Ninja can be installed with Chocolatey.

From a console running with administrative privileges, create a `build_ninja` folder and run `cmake .. -G Ninja`. It is not required to build, just configure CMake. Of course, if you don't care about using the Visual Studio IDE, you can use this folder as your main build tree if you want, building with `cmake --build . -j`

An odd thing is that the console needs administrative privileges to create the shortcut to `compile_commands.json` in the source tree. You have four options: 1) run your prompt with admin privileges, 2) [set your repo folder's permissions to share](https://answers.microsoft.com/en-us/windows/forum/all/you-need-permission-to-perform-this-action-help/38dc9b82-522c-4bdd-a834-3fed96b78069), 3) manually copy the file from the build tree to the source tree, or 4) temporarily modify `.pre-commit-config.yaml` to call clang-tidy with the arg `-p=build_ninja`.

If you re-run CMake on your Visual Studio build tree, you will need to re-run CMake on your Ninja build tree before you can run clang-tidy again.

### Install tools

#### Linux

`pip install pre-commit cpplint clang-format`

`brew install llvm cppcheck`

Installing LLVM brings in clang-tidy and clang-format, however you need the updated version of clang-format, currently on pip but now brew.

#### Windows

`pip install pre-commit cpplint`

`choco install llvm cppcheck ninja`

### Pre-commit

Pre-commit is a python program designed to pull public linting tools hosted online. It uses git hooks. It is not limited to git's pre-commit hook. People and organizations will host their own linting tools for you to use. Isn't that nice? 

Pre-commit's primary intended use case is to force you to pass linting before you can commit. Using this workflow, which is optional for this project, you would run `pre-commit install` in your repo. Then when you run `git commit` in the future, it will automatically run the pre-commit checks first. Pro-tip, `pre-commit unistall` will stop that behavior.

You can also run it any time with `pre-commit run --all-files`. Be careful with this--it's best to have your files at least staged because clang-format will happily auto-format your unstaged files. You can temporarily disable clang-format auto-formatting by commenting-out the `-i` argument. You can also run specific hooks with `pre-commit run --all-files <hook>`. And you can pass in specific files with `pre-commit run --files <file1> [<file2> ...]`.

Pre-commit is configured in the `.pre-commit-config.yaml`  file at the source root.

Besides the C++ linting tools, there are a handful of other pre-commit hooks used by this project for various formatting and linting tasks.
