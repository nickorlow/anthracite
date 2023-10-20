.PHONY: format lint build build-release build-docker run debug

build-supplemental:
	cd ./build && ./version.sh && python3 ./error_gen.py

build: build-supplemental
	g++ main.cpp --std=c++20 -g -o ./anthracite 

build-release: build-supplemental
	g++ main.cpp --std=c++20 -O3 -march=native -o ./anthracite

build-docker:
	docker build . -t anthracite

run: build
	./anthracite 8080

run-test: build
	./anthracite 8080 ./test_www

debug: build
	gdb --args ./anthracite 8080

debug-test: build
	gdb --args ./anthracite 8080 ./test_www

format:
	clang-format *.cpp -i

lint:
	clang-tidy *.cpp 

lint-fix:
	clang-tidy *.cpp -fix -fix-errors
