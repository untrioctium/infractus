/**
 * @file LoggingSystem.hpp
 * @brief Contains the class LoggingSystem
 */

#include <fstream>
#include <ctime>
#include <stdio.h>
#include <stdarg.h>

/**
 * @brief Writes a formatted message to the log.
 * @param level The message's level.
 * @param message The format string for the message to write.
 * @param ... The arguments for the format string.
 */
#define writeLogf( level, message, ... ) realWriteLog( __PRETTY_FUNCTION__, __FILE__, __LINE__, level, message, __VA_ARGS__ )

/**
 * @brief Writes a message to the log.
 * @param level The message's level.
 * @param message The message to write.
 *
 * Use this instead of writeLogf if there are no format arguments,
 * else the compiler will probably compain at you.
 */
#define writeLog( level, message ) realWriteLog( __PRETTY_FUNCTION__, __FILE__, __LINE__, level, message )

/** A class that provides XML based logging for the application. */
class LoggingSystem
{
public:

	/** Indicates the severity of the message written. */
	enum MessageLevel
	{
		Info,    ///< The message is just information.
		Warning, ///< The message indicates a warning.
		Error,   ///< The message indicates a serious error
		Critical ///< The message indicates a serious error that cannot be recovered from.
	};

	/** Default constructor that sets up the internal XML tab depth. */
	LoggingSystem();

	/** Destructor that finishes the log file and closes it. */
	~LoggingSystem();

	/**
	 * @brief Sets up the LoggingSystem and opens the log.
	 * @param filename The file to write to.
	 * @param applicationName The name of the currently running application.
	 */
	void init( std::string filename, std::string applicationName );

	/**
	 * @brief Actual implementation of writing to the log file.
	 * @param function The function requesting the write.
	 * @param file The source file containing the write request.
	 * @param line The actual line where the request appears.
	 * @param level The level of the message.
	 * @param message The message to write.
	 * @param ... The format string for the message.
	 *
	 * This function is "wrapped" by the writeLog() and writeLogf() macros
	 * so that the \a function, \a file, and \a line parameters can be
	 * automatically provided at compile time.  To write to the log, call
	 * these macros like they were member functions of the LoggingSystem class, e.g.:
	 * @code
	 * 	myLog.writeLogf(LoggingSystem::Info, "Hello, world! 2 + 2 = %d", 4);
	 * @endcode
	 */
	void realWriteLog( std::string function, std::string file, int line, 
		LoggingSystem::MessageLevel level, const char* message, ... );

private:

	std::string td() { return std::string(tabDepth, '\t'); }

	int tabDepth;

	static const char* levelNames[];
	std::ofstream logStream;

	char tempBuffer[128];
};
