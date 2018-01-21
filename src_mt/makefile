CC = gcc
CXX = g++ 
INCLUDE = -I/usr/local/unixODBC/include
BIN = 
LIB = -lm -lexpat -L/usr/local/unixODBC/lib -lodbc -L/d2/wangyingbin/mylib/lib -lmylib
CFLAGS = -Wall -O2 -D_FILE_OFFSET_BITS=64
CXXFLAGS = -Wall -O2 -D_FILE_OFFSET_BITS=64 -DSIZEOF_LONG_INT=8
LDFLAGS = 

TARGET = usercf

all: $(TARGET)

usercf: usercf.o main.o Timer.o
	$(CXX) -o $@ $^ $(LIB)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) $< $(INCLUDE)

%.o : %.c	
	$(CXX) -c $(CFLAGS) $< $(INCLUDE)

clean :
	$(RM) $(TARGET) *.o core
