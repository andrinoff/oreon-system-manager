# Oreon System Manager

An all-in-one system management GUI for Fedora-based Linux distributions, built with Qt6. Built for Oreon 11.

## Requirements

- Qt6 (Widgets, Core, Concurrent)
- CMake 3.16+
- GCC or Clang with C++17 support
- `dnf` and `pkexec` (runtime, Fedora/RHEL-based systems)
- `docker` (optional, for container management)
- `distrobox` (optional, for Distrobox management)

## Building

Install dependencies:

```bash
sudo dnf install qt6-qtbase-devel cmake gcc-c++
```

Build:

```bash
./build.sh
```

Or manually:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
```

Run:

```bash
./build/oreon-system-manager
```

## License

This project is protected by the See [LICENSE](LICENSE).
