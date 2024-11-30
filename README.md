# 3DMMEx

3DMMEx is a fork of [Microsoft 3D Movie Maker](https://github.com/microsoft/Microsoft-3D-Movie-Maker) that aims to preserve the classic 3D Movie Maker experience while adding minor enhancements for an improved user experience.

![3D Movie Maker](img/3dmovie.jpg?raw=true)

3DMMEx builds upon the amazing work of the [3DMMForever](https://github.com/foone/3DMMForever) project. It is designed to maintain compatibility with 3DMMForever, with many of the changes designed to be contributed back upstream whenever possible.

## Build

### Requirements

* Visual Studio 2022
  * Desktop development with C++ workload
  * Clang (optional)
* CMake 3.22+
* WiX (optional: required to build MSI packages)

### Building

Use CMake to build the project. The project includes a CMakePresets.json file that specifies [build presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html).

* Open the Developer Command Prompt for Visual Studio 2022 (or use a tool such as [VCVars](https://github.com/bruxisma/VCVars) to add the build tools to your path)
* Run `cmake --preset <preset-name>`:
  * Debug build: `x86-msvc-debug`
    * For local development and testing, use the Debug build. It has additional runtime checking that helps to identify bugs.
  * Release build: `x86-msvc-relwithdebinfo`
  * Clang builds: Choose a `x86-clang-*` preset to compile with Clang instead of the Microsoft Visual C++ compiler
* Run `cmake --build build\<preset-name> --target install`
  * The `--target install` is optional but recommended. This will create a directory `dist\<preset-name>` with the directory layout required for 3DMM to run.
  * Kauai development tools (eg. Chelp, Ched) are not built by default.
* To build a release package, run `cmake --build build\<preset-name> --target dist`
  * A ZIP archive containing a portable install is created in the build directory.
  * If WiX is installed, an MSI package is also created.

## Legal stuff

The following sections have been carried over from the original 3D Movie Maker [GitHub repository](https://github.com/microsoft/Microsoft-3D-Movie-Maker) released by Microsoft in May 2022.

### Code cleanup

This code was restored from the Microsoft corporate archives and cleared for release.

* Developer names and aliases were removed, with the exception of current employees who worked on the
  original release who consented to keeping their names in place
* The archive consisted of several CDs, some of which were for alternate builds or products, and
  have been excluded
* The code does not build with today's engineering tools, and is released as-is.

### Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft
trademarks or logos is subject to and must follow
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.

This repo includes a build from 1995 of BRender from Argonaut software. Approval to open source BRender as MIT was given in an email from Jez San, former CEO of Argonaut. Other versions of BRender exist at <https://github.com/foone/BRender-v1.3.2> and <https://github.com/foone/BRender-1997> Thanks to Jez and the whole BRender team for their hard work on this amazing engine. A full historical list of BRender contributors is available at <https://github.com/foone/BRender-v1.3.2/blob/main/README.md>

This repo does NOT include the SoftImage SDK "./DKIT" from 1992.

Jez also offered this interesting BRender anecdote in an email:

```
When Sam Littlewood designed BRender, he didn’t write the code. And then document it.  
The way most things were built at the time.
First, he wrote the manual.  The full documentation
That served as the spec.  Then the coding started.
```
