CXX=g++
CXXFLAGS=-std=c++0x -Wall -Wextra -Werror -pedantic -pedantic-errors -Wno-unused-parameter -g -ldmlite -ljansson -lcurl

.PHONY: all
all: plugin_rucio.so test

.PHONY: clean
clean:
	rm -f *.o plugin_rucio.so test

plugin_rucio.so: plugin_rucio.h plugin_rucio.cc rucio_catalog.h rucio_catalog.cc rucio_connect.h rucio_connect.cc
	$(CXX) $(CXXFLAGS) -fPIC -shared plugin_rucio.cc rucio_catalog.cc rucio_connect.cc -o plugin_rucio.so

test: test.cc
	$(CXX) $(CXXFLAGS) test.cc -o test
