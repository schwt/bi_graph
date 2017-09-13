CC   	= gcc
CXX 	= g++ 
INCLUDE = 
LIB 	= -lm -L../../clib/a -lmylib
CFLAGS  = -Wall -O2 -D_FILE_OFFSET_BITS=64 
CXXFLAGS = -Wall -O2 -D_FILE_OFFSET_BITS=64 -DSIZEOF_LONG_INT=8 -Wno-deprecated
LDFLAGS = 

TARGET = spear

all: $(TARGET)

spear: bi_graph.o main.o
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIB)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) $< $(INCLUDE)

%.o : %.c	
	$(CXX) -c $(CFLAGS) $< $(INCLUDE)

clean :
	$(RM) $(TARGET) *.o core
