CXX = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -std=c++17

CXX_SRCS = cache_sim.cpp main.cpp
CXX_OBJS = $(CXX_SRCS:.cpp=.o)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $*.cpp -o $*.o

csim : $(CXX_OBJS)
	$(CXX) -o $@ $(CXX_OBJS)

.PHONY: solution.zip
solution.zip :
	rm -f $@
	zip -9r $@ *.c *.cpp *.h README.txt

clean :
	rm -f csim *.o

# Generate header file dependencies
depend :
	$(CXX) $(CXXFLAGS) -M $(CXX_SRCS) > depend.mak

depend.mak :
	touch $@

include depend.mak
