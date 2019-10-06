#include <SDL2/SDL.h>
//#include <SDL_console/SDL_console.h>
#include <GL/glew.h>
#include <stdio.h>
#include "Global.hpp"

#include <boost/foreach.hpp>

#include "Array.hpp"

#include "Singleton.hpp"
#include "LoggingSystem.hpp"
#include "InputSystem.hpp"
#include "FileSystem.hpp"
#include "GraphicsSystem.hpp"
#include "ConfigSystem.hpp"

#include "InfractusProgram.hpp"
#include "ProgramManager.hpp"

#include "Infractus.hpp"
#include <libcompute.hpp>

#include <iostream>

//#include "InfractusConsole.hpp"

using namespace std;

char* path;

Infractus::~Infractus()
{
	delete inputSystem;
	delete graphicsSystem;
	delete configSystem;
	delete fileSystem;
	delete loggingSystem;
}



bool Infractus::init( int argc, char** argv )
{
	this->progArgc = argc;
	this->progArgv = argv;
	
	this->exitRequested = false;
	inspectMode = false;
	
	try
	{
		loggingSystem = &Singleton<LoggingSystem>::instance();
		loggingSystem->init( "infractus.log.xml", "Infractus" );
	
		fileSystem = &Singleton<FileSystem>::instance();
		fileSystem->init();

		configSystem = &Singleton<ConfigSystem>::instance();
		configSystem->init();
		configSystem->loadConfig("infractus.xml");

		graphicsSystem = &Singleton<GraphicsSystem>::instance();
		graphicsSystem->init();
		graphicsSystem->setDrawingMode(GraphicsSystem::TwoD);

		inputSystem = &Singleton<InputSystem>::instance();
		inputSystem->init();
		
		programManager = new ProgramManager("./programs/");
		

	}
	catch( std::exception& e )
	{
		loggingSystem->writeLog( LoggingSystem::Critical, e.what() );

		return false;
	}
	srand(time(NULL));
	
	ScreenInfo screenInfo = graphicsSystem->getScreenInfo();

	SDL_Rect consoleRect;
	consoleRect.x = consoleRect.y = 0;
	consoleRect.w = screenInfo.w;
	consoleRect.h = configSystem->getConfigPtree("infractus.xml").get<int>("infractus.graphics.console_height");

	//consoleBuffer = graphicsSystem->requestBufferSurface( consoleRect.w, consoleRect.h );
	
	//console = CON_Init("images/ConsoleFont.bmp", consoleBuffer, 100, consoleRect);
	//BitFont* font = DT_FontPointer( console->FontNumber );
	//SDL_SetColorKey( font->FontSurface, SDL_SRCCOLORKEY, SDL_MapRGB( font->FontSurface->format,  255, 0, 255 ) );
	//CON_SetExecuteFunction(console, Infractus::consoleCommandHandlerWrapper);

	consolePrompt = "] ";
	//setConsolePrompt();
	//CON_Topmost(console);

	program = NULL;

	loggingSystem->writeLog( LoggingSystem::Info, "about to load" );
	
	try
	{
		program = programManager->createInstance((argc > 1)? argv[1]: "Pickover");
		program->init(false, screenInfo.w, screenInfo.h);
	}
	catch( std::exception& e )
	{
		printf("%s\n", e.what());
		return false;
	}
	
	/*#define INFRACTUS_DEFINE_COMMAND(name, help, minargc)\
		commandMap[#name] = ConsoleCommandInfo(help, minargc, &Infractus::name)
	
	INFRACTUS_DEFINE_COMMAND(echo, "echo strings back to the console", 1);
	INFRACTUS_DEFINE_COMMAND(help, "display this help text", 0);
	INFRACTUS_DEFINE_COMMAND(prompt, "set the console prompt", 1);
	INFRACTUS_DEFINE_COMMAND(inspect, "inspect the active program", 0 );
	INFRACTUS_DEFINE_COMMAND(program_info, "get information on the loaded program or another program", 0);
	INFRACTUS_DEFINE_COMMAND(list_programs, "list all avaliable programs", 0);
	INFRACTUS_DEFINE_COMMAND(refresh_programs, "refresh the program list", 0);
	*/
	return true;

}
/*
void Infractus::printToConsole( const std::string& str )
{
	printf("Console: %s\n", str.c_str());
	//CON_Out( console, "%s", str.c_str());
}*/

void saveFrame( Texture tex, int frame )
{
	static const char* base = "./frames/";
	
	char temp[128];
	sprintf(temp, "%s%04d", base, frame);
	
	std::string path = temp;
	Singleton<GraphicsSystem>::instance().saveTexture( tex, path );
	
	if( frame % 25 == 0 )
	{
		system("mogrify -format jpg -flip -quality 100 ./frames/*.pbm; rm ./frames/*.pbm");
	}
	
}

int Infractus::run()
{
	ScreenInfo screenInfo = graphicsSystem->getScreenInfo();

	#define PROGRAM_TRY_CATCH( name, code ) code;

	//InfractusProgram* deform = programManager->createInstance("Deform");
	//deform->init( false, screenInfo.w, screenInfo.h );
	//deform->setUsedTexture( program->getBufferTexture() );
//int cat = 1;
	while( !this->exitRequested )
	{
		unsigned int dt = graphicsSystem->delta();
		
		inputSystem->queueInputEvents();

		/*if( CON_isVisible(console) )
		{
			for( std::vector<SDL_Event>::iterator i = inputSystem->getQueuedEventsIterator();
				i != inputSystem->getQueuedEventsEnd(); i++ )
			{
				if( CON_Events(&(*i)) == NULL )
				{
					if( i->key.keysym.sym == console->HideKey )
						SDL_EnableKeyRepeat( 0, 0 );
						
					i = inputSystem->removeQueuedEvent(i);
					if( i == inputSystem->getQueuedEventsEnd() ) break;
				}
			}
		}*/
		
		inputSystem->processInputQueue();
		
		if( inputSystem->getKeyState( SDLK_ESCAPE ) == InputSystem::Pressed || inputSystem->quitRequested() )
			exitRequested = true;
		
		/*if( inputSystem->getKeyState( SDLK_BACKQUOTE ) == InputSystem::Released && !CON_isVisible( console ) )
		{
			CON_Show( console );
			SDL_EnableKeyRepeat( 250, 30 );
		}

		if( !CON_isVisible(console) )*/
			PROGRAM_TRY_CATCH("input", program->input())
			
			PROGRAM_TRY_CATCH("run", program->run(dt, 1.0))

		graphicsSystem->drawToTexture(program->getBufferTexture());
		PROGRAM_TRY_CATCH("draw", program->draw());
		graphicsSystem->drawToTexture(Texture());

		//PROGRAM_TRY_CATCH("run", deform->run(dt, 1.0))

		//graphicsSystem->drawToTexture(deform->getBufferTexture());
		//PROGRAM_TRY_CATCH("draw", deform->draw());
		//graphicsSystem->drawToTexture(Texture());
		
		Texture finalTex;
		PROGRAM_TRY_CATCH("getOutput", finalTex = program->getOutput())
		
		//saveFrame( finalTex, cat );
		graphicsSystem->useTexture(finalTex);
		graphicsSystem->drawRectangle(Point(0,screenInfo.h), Point(screenInfo.w, 0));
		graphicsSystem->useTexture(Texture());
		//graphicsSystem->clearToColor( Color(0.0, 0.0, 0.0, 1.0));
		/*CON_DrawConsole(console);
		if( CON_isVisible( console ) )
		{
			SDL_UpdateRect( consoleBuffer, 0, 0, 0, 0 );
			graphicsSystem->setDrawColor( Color(1.0, 1.0, 1.0, .75) );
			graphicsSystem->drawSurface( consoleBuffer, Point(0,0) );
			graphicsSystem->resetDrawColor();
		}*/

		graphicsSystem->render();
		//cat++;
		
		//if( cat == 1501 ) this->exitRequested = true;
	}

	return 0;
}
/*
void Infractus::setConsolePrompt( std::string prompt )
{
	if( prompt.size() == 0 ) prompt = consolePrompt;
	
	char promptStr[prompt.size() + 1];
	sprintf(promptStr, "%s", prompt.c_str());
	//CON_SetPrompt( console, promptStr );
}

void Infractus::consoleCommandHandlerWrapper( ConsoleInformation* console, char* command )
{
	Singleton<Infractus>::instance().consoleCommandHandler( console, command );
}

void Infractus::consoleCommandHandler( ConsoleInformation* console, char* command )
{
	std::string cmd = command;

	std::vector<std::string> tokens = splitLine(cmd);
	std::string cmdName = tokens[0];
	tokens.erase(tokens.begin());
	
	if( inspectMode )
	{
		inspect( std::vector<std::string>( 1, cmd ) );
		return;
	}
	
	if( commandMap.count(cmdName) == 0 )
	{
		//CON_Out(console, "Unknown command '%s'.", cmdName.c_str());
		return;
	}
	
	//ConsoleCommandInfo cmdInfo = commandMap[cmdName];
	if( tokens.size() < cmdInfo.minArgc )
	{
		//CON_Out(console, 
		//	"Too few arguments for command '%s' (was provided %d, requires at least %d).", 
		//	cmdName.c_str(), tokens.size(), cmdInfo.minArgc);
		return;
	}
	
	cmdInfo.function(this, tokens);
}*/

vector<string> Infractus::splitLine( string line )
{
	line += ' ';

	vector<string> out;
	string token;
	bool escapeActive = false;
	bool inQuotes = false;
	BOOST_FOREACH( char ch, line )
	{
		if( ch == ' ' )
		{
			if( token.length() && !inQuotes )
			{
				out.push_back( token );
				token = std::string();
			} else if( inQuotes ) token+=ch;
		}
		else if( ch == '\"' )
		{
			if( escapeActive )
			{
				token += ch;
				escapeActive = false;
			}
			else inQuotes = !inQuotes;
		}
		else if( ch == '\\' )
		{
			if( escapeActive )
				token += ch;
			escapeActive = !escapeActive;
		} else token += ch;
	}

	if( token.length() )
		out.push_back(token);

	return out;
}


int main( int argc, char** argv )
{
	path = argv[0];
	Infractus* infractus = &Singleton<Infractus>::instance();
	if( !infractus->init(argc, argv) )
	{
		printf("lolwut");
		delete infractus;
		return 1;
	}
	int retval = infractus->run();
	delete infractus;
	return retval;
}
