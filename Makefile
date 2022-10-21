CXXFLAGS= -Wall -Wextra -g 
CPPFLAGS= -I ./include 
LDFLAGS= -g -lconfig++
CXX=g++

all: launcher

launcher: launcher.o configuration_parser.o AS.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

launcher.o: src/launcher.cc 
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

configuration_parser.o: src/configuration_parser.cc include/configuration_parser.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

AS.o: src/AS.cpp include/AS.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

clean:
	@rm -f *.o 
	@rm -f launcher

.PHONY: all clean 	