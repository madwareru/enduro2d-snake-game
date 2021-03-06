# Build Instructions

## * Requirements

- [git](https://git-scm.com/)
- [git-lfs](https://git-lfs.github.com/)
- [cmake](https://cmake.org/) **>= 3.11**
- [gcc](https://www.gnu.org/software/gcc/) **>= 4.9** or [clang](https://clang.llvm.org/) **>= 3.8** or [msvc](https://visualstudio.microsoft.com/) **>= 2015**

## * Cloning

```bash
$ git clone --recursive git://github.com/enduro2d/enduro2d-bootstrap.git
$ cd enduro2d-bootstrap
$ git lfs install
$ git lfs pull
```

## * Building

```bash
$ cd your_bootstrap_repository_directory
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build . -- -j8

# also you can generate Xcode project
$ cd your_bootstrap_repository_directory
$ mkdir xcode-build && cd xcode-build
$ cmake -G Xcode ..
$ open enduro2d_snake_game.xcodeproj

# or Visual Studio project
$ cd your_bootstrap_repository_directory
$ mkdir msvc-build && cd msvc-build
$ cmake -G "Visual Studio 15 2017" ..
# in case of you have msbuild in path:
$ MSBuild.exe .\ALL_BUILD.vcxproj -property:Configuration={Debug|Release}
#or if you want to build from within a Visual Studio:
$ start enduro2d_snake_game.sln
```

## * Running

```bash
$ cd your_bootstrap_build_directory
$ ./enduro2d_snake_game
```

## * Links

- CMake: https://cmake.org/
- CMake documentation: https://cmake.org/documentation/
- CMake FAQ: https://gitlab.kitware.com/cmake/community/wikis/FAQ
