OBJS = Engine.o Parameter.o Plugin.o Program.o ProgramDataTypes.o UnixSharedLibrary.o

HEADERDIR = include/libcompute
HEADERS = include/libcompute.hpp $(HEADERDIR)/Engine.hpp $(HEADERDIR)/Exceptions.hpp $(HEADERDIR)/Parameter.hpp \
		  $(HEADERDIR)/Plugin.hpp $(HEADERDIR)/Program.hpp $(HEADERDIR)/ProgramDataTypes.hpp $(HEADERDIR)/SharedLibrary.hpp

SRCPATH = src
		  
LIBNAME = libcompute
VERSION = 1.0.1
LIBPATH = lib

CC = g++
DEBUG = -g
CFLAGS = -Wall -I./include -c -fPIC $(DEBUG)
LFLAGS = -Wall --shared -Wl,--out-implib,$(LIBPATH)/$(LIBNAME).a $(DEBUG) -o $(LIBPATH)/$(LIBNAME).dll


libcompute: $(OBJS) $(HEADERS)
	$(CC) $(LFLAGS) $(OBJS)
	
Engine.o: $(SRCPATH)/Engine.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(SRCPATH)/Engine.cpp
	
Parameter.o: $(SRCPATH)/Parameter.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(SRCPATH)/Parameter.cpp
	
Plugin.o: $(SRCPATH)/Plugin.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(SRCPATH)/Plugin.cpp

Program.o: $(SRCPATH)/Program.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(SRCPATH)/Program.cpp

ProgramDataTypes.o: $(SRCPATH)/ProgramDataTypes.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(SRCPATH)/ProgramDataTypes.cpp
	
UnixSharedLibrary.o: $(SRCPATH)/UnixSharedLibrary.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(SRCPATH)/UnixSharedLibrary.cpp
	
clean:
	rm $(OBJS) $(LIBNAME).so.$(VERSION)
