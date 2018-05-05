OBJS = GLSLComputeEngine.o
HEADERS = glUniform.hpp
		  
LIBNAME = plugin.dll

CC = g++
DEBUG = -g
CFLAGS = -Wall -I../../libcompute/include -c -fPIC $(DEBUG)
LFLAGS = -L../../libcompute/lib -Wall -lopengl32 -lglew32 -lglu32 -lcompute -shared $(DEBUG) -o $(LIBNAME)


GLSLComputeEngine: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) 
	
GLSLComputeEngine.o: GLSLComputeEngine.cpp $(HEADERS)
	$(CC) $(CFLAGS) GLSLComputeEngine.cpp
	
clean:
	rm $(OBJS) $(LIBNAME)
