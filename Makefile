INCLUDES = -I /usr/local/include -I include/PEGTL/include
DOWNLOAD = wget

default: cli

all: tests cli

cli:
	c++ -std=c++1z $(INCLUDES) src/main.cxx -o bin/pglogs

cli-debug:
	c++ -g -std=c++1z $(INCLUDES) src/main.cxx -o bin/pglogs

download_deps:
	cd include && git clone --depth 1 https://github.com/cieplak/PEGTL.git

download_test_deps:
	cd test
	$(DOWNLOAD) https://github.com/philsquared/Catch/releases/download/v1.7.0/catch.hpp

tests: download_test_deps
	c++ -std=c++1z -I . $(INCLUDES) src/main.cxx test/tests.cpp -o bin/tests
	./bin/tests

clean:
	rm bin/pglogs
	rm bin/tests
	rm test/catch.hpp
