#include "Global.hpp"
#include "LoggingSystem.hpp"

const char* LoggingSystem::levelNames[4] = {"Info", "Warning", "Error", "Critical"};

LoggingSystem::LoggingSystem(void)
{
	tabDepth = 0;
}

LoggingSystem::~LoggingSystem(void)
{
	writeLog( LoggingSystem::Info, "Closing log file due to application exit." );
	
	tabDepth--;
	logStream << td() << "</LogEntries>" << std::endl;


	logStream << "</Project>" << std::endl;

	logStream.close();
}

void LoggingSystem::init( std::string filename, std::string project_name )
{
	logStream.exceptions( std::ofstream::failbit | std::ofstream::badbit );
	logStream.open( filename.c_str() );

	logStream << "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>" << std::endl;
	logStream << "<?xml-stylesheet type=\"text/xsl\" href=\"infractus.log.xsl\" ?>" << std::endl << std::endl;

	logStream << "<Project>" << std::endl;

	tabDepth++;
	logStream << td() << "<Name>" << project_name << "</Name>" << std::endl;
	logStream << td() << "<Compiled>" << __DATE__ " " __TIME__ << "</Compiled>" << std::endl;
	logStream << td() << "<LogEntries>" << std::endl;
	tabDepth++;

	writeLog( LoggingSystem::Info, "Opened logging." );

	logStream.flush();
}

void LoggingSystem::realWriteLog(std::string function, std::string file, int line, LoggingSystem::MessageLevel level, const char* message, ...)
{
	va_list args;
	va_start( args, message );

	vsprintf( tempBuffer, message, args ); 

	logStream << td() << "<LogEntry>" << std::endl;
	tabDepth++;

	logStream << td() << "<ClassName><![CDATA[";

	size_t colon_pos = function.find("::");
	//size_t space_pos = function.find(" ");
	//size_t paren_pos = function.find("(");

	//if( space_pos =

	if( colon_pos != std::string::npos )
	{
		logStream << function.substr( 0, colon_pos );
		function = function.substr( colon_pos + 2, function.size() );
	}

	logStream << "]]></ClassName>" << std::endl;

	logStream << td() << "<FuncName><![CDATA[" << function << "]]></FuncName>" << std::endl;
	logStream << td() << "<DebugType>" << levelNames[level] << "</DebugType>" << std::endl;

	time_t the_time = time(0);
	std::string str_time = ctime( &the_time );
	str_time = str_time.substr( 0, str_time.size() - 1 );

	logStream << td() << "<Timestamp>" << str_time << "</Timestamp>" << std::endl;
	logStream << td() << "<Info><![CDATA[" << tempBuffer << "]]></Info>" << std::endl;
	logStream << td() << "<Line>" << line << "</Line>" << std::endl;
	logStream << td() << "<File>" << file << "</File>" << std::endl;

	tabDepth--;

	logStream << td() << "</LogEntry>" << std::endl;

	logStream.flush();

}
