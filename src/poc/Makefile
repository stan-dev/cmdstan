CXX = clang++
INCLUDES = -I arrow/cpp/release/lib/include -I ../../lib/CLI11-1.9.1
CXXFLAGS = -O3 -Wall -c -std=c++17 -stdlib=libc++ $(INCLUDES)
LDFLAGS = -std=c++1y -stdlib=libc++ -L arrow/cpp/release/release -Wl,-rpath,arrow/cpp/release/release -larrow -lparquet -larrow_bundled_dependencies
EXE = arrow_poc

all: $(EXE)

$(EXE): arrow_poc.o
	$(CXX) $(LDFLAGS) $^ -o $@

$%.o : %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ 

.PHONY: print-%
print-%  : ; @echo $* = $($*)


