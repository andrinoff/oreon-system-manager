.PHONY: all build release fmt fmt-check test run clean install uninstall help

BINARY := oreon-system-manager
PREFIX ?= /usr
DESTDIR ?=

help:
	@echo "Available targets:"
	@echo "  make build       - Debug build (cargo build)"
	@echo "  make release     - Release build with LTO (cargo build --release)"
	@echo "  make fmt         - Format code (cargo fmt --all)"
	@echo "  make fmt-check   - Check formatting without modifying (cargo fmt --check)"
	@echo "  make test        - Run unit tests (cargo test)"
	@echo "  make run         - Build and run the app"
	@echo "  make clean       - Remove build artifacts"
	@echo "  make install     - Install binary, icon, and desktop file to PREFIX (default: /usr)"
	@echo "  make uninstall   - Remove installed files from PREFIX"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX=DIR       - Installation prefix (default: /usr)"
	@echo "  DESTDIR=DIR      - Staging directory for packaging"

all: build

build:
	cargo build

release:
	cargo build --release

fmt:
	cargo fmt --all

fmt-check:
	cargo fmt --all -- --check

test:
	cargo test

run: build
	cargo run

clean:
	cargo clean

install: release
	install -Dm755 target/release/$(BINARY) \
		$(DESTDIR)$(PREFIX)/bin/$(BINARY)
	install -Dm644 assets/logo.png \
		$(DESTDIR)$(PREFIX)/share/icons/hicolor/200x200/apps/$(BINARY).png
	install -Dm644 packaging/$(BINARY).desktop \
		$(DESTDIR)$(PREFIX)/share/applications/$(BINARY).desktop
	@echo "Installed to $(PREFIX)"
	@echo "Run 'gtk-update-icon-cache' to refresh the icon cache"

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BINARY)
	rm -f $(DESTDIR)$(PREFIX)/share/icons/hicolor/200x200/apps/$(BINARY).png
	rm -f $(DESTDIR)$(PREFIX)/share/applications/$(BINARY).desktop
	@echo "Uninstalled from $(PREFIX)"
