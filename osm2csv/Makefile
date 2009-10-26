CXX=g++
CXXFLAGS=-W -Wall -ansi -pedantic -g -Wno-deprecated 
LDFLAGS=-lexpat -lbz2
EXEC=osm2csv

all: $(EXEC)

osm2csv: parameters.o main.o osmreader.o bz2reader.o stdinreader.o csvwriter.o
	$(CXX) -o $@ $^ $(LDFLAGS)

main.o: main.cc main.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

parameters.o: parameters.cc main.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

osmreader.o: osmreader.cc osmreader.h reader.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

bz2reader.o: bz2reader.cc bz2reader.h reader.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

stdinreader.o: stdinreader.cc stdinreader.h reader.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

csvwriter.o: csvwriter.cc csvwriter.h writer.h
	$(CXX) -o $@ -c $< $(CXXFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)

