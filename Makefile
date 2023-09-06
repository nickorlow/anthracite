.PHONY: format

build:
	g++ main.cpp -g -o anthracite

build-release:
	g++ main.cpp -O2 -o anthracite

build-docker:
	docker build . -t anthracite

run: build
	./anthracite

format:
	clang-format *.cpp -i
