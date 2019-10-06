OBJS = Infractus.o InputSystem.o LoggingSystem.o GraphicsSystem.o ConfigSystem.o FileSystem.o ProgramManager.o
CC = clang
DEBUG = -g
LIBS = `sdl2-config --libs` -lcompute -lstdc++ -lSDL2_image -lGL -lGLU -lGLEW -lstdc++fs -llua5.2 -lm
CFLAGS = -Wall -c -I../libcompute/include -I./include -I../extern/sol2/single/include -I/usr/include/lua5.2 `sdl2-config --cflags` -fPIC --std=c++2a $(DEBUG)
LFLAGS = -Wall -L../libcompute/lib  $(LIBS) $(DEBUG)

infractus: $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o infractus

InputSystem.o: InputSystem.cpp include/InputSystem.hpp include/LoggingSystem.hpp include/Global.hpp
	$(CC) $(CFLAGS) InputSystem.cpp

LoggingSystem.o: LoggingSystem.cpp include/LoggingSystem.hpp include/Global.hpp
	$(CC) $(CFLAGS) LoggingSystem.cpp

GraphicsSystem.o: GraphicsSystem.cpp include/GraphicsSystem.hpp include/LoggingSystem.hpp include/Global.hpp
	$(CC) $(CFLAGS) GraphicsSystem.cpp

ConfigSystem.o: ConfigSystem.cpp include/ConfigSystem.hpp
	$(CC) $(CFLAGS) ConfigSystem.cpp

FileSystem.o: FileSystem.cpp include/FileSystem.hpp
	$(CC) $(CFLAGS) FileSystem.cpp

ProgramManager.o: ProgramManager.cpp include/InfractusProgram.hpp include/ProgramManager.hpp include/LuaInfractusProgram.hpp include/LuaProgramLoader.hpp include/LuaHelpers.hpp
	$(CC) $(CFLAGS) ProgramManager.cpp

Infractus.o: Infractus.cpp include/FileSystem.hpp include/InfractusProgram.hpp include/Singleton.hpp include/Infractus.hpp include/LoggingSystem.hpp include/InputSystem.hpp include/GraphicsSystem.hpp include/Global.hpp include/InfractusConsole.hpp include/Array.hpp
	$(CC) $(CFLAGS) Infractus.cpp

clean:
	rm *.o infractus
