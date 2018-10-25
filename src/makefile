CC   	= gcc
CXX 	= g++ 
INCLUDE = 
LIB 	= -lm -L../clib/a -lmylib -lpthread
CFLAGS  = -Wall -O2 -D_FILE_OFFSET_BITS=64 
CXXFLAGS = -Wall -O2 -D_FILE_OFFSET_BITS=64 -DSIZEOF_LONG_INT=8 -Wno-deprecated
LDFLAGS = 

TARGET = graph mt_graph check_struct

all: $(TARGET)

graph: bi_graph.o main_st.o
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIB)

mt_graph: mt_graph.o main.o
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIB)

check_struct: check_struct.o 
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIB)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) $< $(INCLUDE)

%.o : %.c
	$(CXX) -c $(CFLAGS) $< $(INCLUDE)

clean :
	$(RM) $(TARGET) *.o core
