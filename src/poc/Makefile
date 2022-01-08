CXX = clang++
INCLUDES = -I /usr/local/include/arrow -I /usr/local/include/parquet -I ../../lib/CLI11-1.9.1
CXXFLAGS = -O3 -Wall -c -std=c++1y -stdlib=libc++ $(INCLUDES)
LDFLAGS =  -std=c++1y -stdlib=libc++ -L /usr/local/lib -larrow -lparquet -larrow_bundled_dependencies
EXE = arrow_poc

all: $(EXE)

$(EXE): arrow_poc.o
	$(CXX) $(LDFLAGS) $^ -o $@

$%.o : %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ 

