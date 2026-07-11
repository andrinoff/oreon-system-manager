# Oreon System Manager

An all-in-one system management GUI for Fedora-based Linux distributions, built with Rust and GTK4. Built for Oreon 11.

## Features

- **Package management** — search, install, and remove packages via `dnf`
- **Repository management** — list and toggle repositories via `dnf config-manager`
- **Container management** — manage Docker and Distrobox containers
- **Driver detection** — detect hardware and install appropriate drivers
- **Theme support** — Breeze (light), Breeze Dark, Catppuccin Mocha, and Nord themes

## Requirements

- Rust 1.70+ (stable)
- GTK4 development files
- `pkg-config`
- `dnf` and `pkexec` (runtime, Fedora/RHEL-based systems)
- `docker` (optional, for container management)
- `distrobox` (optional, for Distrobox management)

## Building

Install dependencies:

```bash
sudo dnf install gtk4-devel pkg-config
```

Build:

```bash
./build.sh
```

Or manually:

```bash
cargo build --release
```

Run:

```bash
./target/release/oreon-system-manager
```

## Development

```bash
# Debug build
cargo build

# Run tests
cargo test

# Run the app
cargo run
```

## License

Copyright (C) 2026 Oreon HQ. This program is licensed under GPL-3.0. See [LICENSE](LICENSE).
