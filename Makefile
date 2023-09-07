.PHONY: format lint build build-release build-docker run debug

build:
	g++ main.cpp -g -o anthracite

build-release:
	g++ main.cpp -O2 -o anthracite

build-docker:
	docker build . -t anthracite

run: build
	./anthracite 8080

debug: build
	gdb --args ./anthracite 8080

format:
	clang-format *.cpp -i

lint:
	clang-tidy *.cpp 

lint-fix:
	clang-tidy *.cpp -fix -fix-errors
