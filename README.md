# Vivianite-engine

## What is it?
Vivianite is an OpenGL Game engine written in C++ that uses a Forward+ rendering engine. It's structured with Modules that can be used by user-scripts to communicate with the engine and modify the game.

## Why though?
Because I was bored and felt like it.

## Is there any reason this exists?
Not really, I just wanted to make a game engine.

## Okay, but is there any reason to use it?
Yes, it's set to use an easy-to-use scripting language, [CUTE](https://github.com/Ferriit/CUTE-lang), which transpiles directly to C++, which makes it compile with the main runtime and may improve speed compared to some JIT-Compiled or Interpreted languages used in game engines. It also features C++ for more advanced developers, and custom GLSL support. The engine isn't hard to modify either.

## Cool. How do I install it?
The installation method varies from platform to platform (however, cross-compilation is planned for compiling the games themselves).

### Windows

#### Dependencies
- CMake
- Git
- Visual Studio 2022 with the "Desktop development with C++" workload

#### Commands

From the repository root:

```powershell
git clone --recursive https://github.com/ferriit/vivianite-engine
cd vivianite-engine

cmake -B build
cmake --build build --config Release
```

The generated binary is either located at `build\\Release\\Vivianite.exe` or `build\\Vivianite.exe`.

### Linux, MacOS and other POSIX-based systems
#### Dependencies
- Make
- CMake
- Git
- Ninja
- C++ 20 (or higher) compiler

#### Commands
```
git clone --recursive https://github.com/ferriit/vivianite-engine
cd vivianite-engine
make release
```
The generated binary is now located at `build/Vivianite`
