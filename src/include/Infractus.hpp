
#include <boost/function.hpp>
class Infractus
{
public:
	Infractus() {}
	~Infractus();

	bool init(int argc, char** argv);
	int run();

	//static void consoleCommandHandlerWrapper( ConsoleInformation* console, char* command );
	//void consoleCommandHandler( ConsoleInformation* console, char* command );
	//static int luaPrint( lua_State* L );
	
	//void printToConsole( const std::string& str );

private:

	bool exitRequested;

	static std::vector<std::string> splitLine( std::string line );

	typedef boost::function<void (Infractus* const, const std::vector<std::string>&)> ConsoleCommand;
	
	std::string consolePrompt;
	void setConsolePrompt( std::string prompt = std::string() );
	
	/*struct ConsoleCommandInfo
	{
		std::string helpText;
		unsigned int minArgc;
		ConsoleCommand function;
		
		ConsoleCommandInfo( const char* help, unsigned int ac, ConsoleCommand func )
		: helpText(help)
		, minArgc(ac)
		, function(func) {}
		
		ConsoleCommandInfo() {}
	};
	
	std::map<std::string, ConsoleCommandInfo> commandMap;
	
	#define INFRACTUS_CONSOLE_COMMAND(name) void name( const std::vector<std::string>& args )
	#define INFRACTUS_CONSOLE_COMMAND_IMPL(name) INFRACTUS_CONSOLE_COMMAND(Infractus::name)
	
	INFRACTUS_CONSOLE_COMMAND(help);
	INFRACTUS_CONSOLE_COMMAND(echo);
	INFRACTUS_CONSOLE_COMMAND(prompt);
	INFRACTUS_CONSOLE_COMMAND(program_info);
	INFRACTUS_CONSOLE_COMMAND(list_programs);
	INFRACTUS_CONSOLE_COMMAND(refresh_programs);

	INFRACTUS_CONSOLE_COMMAND(inspect);*/
	bool inspectMode;
	
	ProgramManager* programManager;
	InfractusProgram* program;

	LoggingSystem* loggingSystem;
	GraphicsSystem* graphicsSystem;
	ConfigSystem* configSystem;
	InputSystem* inputSystem;
	FileSystem* fileSystem;
	
	//ConsoleInformation* console;
	//SDL_Surface* consoleBuffer;

	int progArgc;
	char** progArgv;

};
