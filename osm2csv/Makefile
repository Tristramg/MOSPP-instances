CXX=g++
CXXFLAGS=-W -Wall -ansi -pedantic -O3 -Wno-deprecated 
LDFLAGS=-lexpat
EXEC=osm2csv

all: $(EXEC)

osm2csv: parameters.o main.o
	$(CXX) -o $@ $^ $(LDFLAGS)

main.o: main.cc main.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

parameters.o: parameters.cc main.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)

