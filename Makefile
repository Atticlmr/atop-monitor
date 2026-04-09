.PHONY: all build install clean uninstall

all: build

build:
	mkdir -p build
	cd build && cmake .. && make -j$$(nproc)

install: build
	cd build && sudo make install

uninstall:
	cd build && sudo make uninstall

clean:
	rm -rf build

# Quick run for testing
run: build
	./build/atop-monitor
