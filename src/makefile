OBJS = Infractus.o InputSystem.o LoggingSystem.o GraphicsSystem.o ConfigSystem.o FileSystem.o ProgramManager.o
CC = g++
DEBUG = -g
LIBS = `sdl-config --libs` -lcompute -lstdc++ -lSDL_image -lglew32 -lglu32 -lopengl32 -lstdc++fs -lluabind -llua
CFLAGS = -Wall -c -I../libcompute/include -I./include `sdl-config --cflags` -fPIC $(DEBUG)
LFLAGS = -Wall -L/mingw64/lib -L/mingw32/lib -L../libcompute/lib $(LIBS) $(DEBUG)

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

ProgramManager.o: ProgramManager.cpp include/InfractusProgram.hpp include/ProgramManager.hpp
	$(CC) $(CFLAGS) ProgramManager.cpp

Infractus.o: Infractus.cpp include/FileSystem.hpp include/InfractusProgram.hpp include/Singleton.hpp include/Infractus.hpp include/LoggingSystem.hpp include/InputSystem.hpp include/GraphicsSystem.hpp include/Global.hpp include/InfractusConsole.hpp include/Array.hpp
	$(CC) $(CFLAGS) Infractus.cpp

clean:
	rm *.o infractus
