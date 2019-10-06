OBJS = GLSLComputeEngine.o
HEADERS = glUniform.hpp
		  
LIBNAME = plugin.so

CC = clang
DEBUG = -g
CFLAGS = -Wall -I../../libcompute/include -c -fPIC --std=c++2a $(DEBUG)
LFLAGS = -Wall -rdynamic -shared -lstdc++ $(DEBUG) -o $(LIBNAME)


GLSLComputeEngine: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS)
	
GLSLComputeEngine.o: GLSLComputeEngine.cpp $(HEADERS)
	$(CC) $(CFLAGS) GLSLComputeEngine.cpp
	
clean:
	rm $(OBJS) $(LIBNAME)