# Ginga

Ginga is a C++20 / Qt 6 terminal project, starting with a minimal GUI smoke test.

## Milestone v0.1

- [x] GitHub repository created (user confirmed)
- [ ] Local Git repository connected
- [ ] CMake configured successfully on Windows
- [ ] Qt window launches and closes correctly on Windows
- [ ] Release build and launch verified on Windows

## Configure (PowerShell, Visual Studio developer environment)

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.12.0/msvc2022_64"
cmake --build build --config Release
.\build\Release\ginga.exe
```

This first version has no PTY, terminal parser, or network code.
