#include "Global.hpp"
#include "LoggingSystem.hpp"
#include "ConfigSystem.hpp"
#include <boost/property_tree/xml_parser.hpp>

#include "Singleton.hpp"

void ConfigSystem::init()
{
	Singleton<LoggingSystem>::instance().writeLog(LoggingSystem::Info, "Configuration system initialized");
}

bool ConfigSystem::isLoaded(const std::string& name)
{
	return configFiles_.find( name ) != configFiles_.end();
}

bool ConfigSystem::loadConfig(const std::string& name)
{
	if( isLoaded( name ) )
	{
		Singleton<LoggingSystem>::instance().writeLogf( LoggingSystem::Warning, "Trying to open %s when it is already open.", name.c_str() );
		return true;
	}

	ptree configTree;
	read_xml( name, configTree );

	Singleton<LoggingSystem>::instance().writeLogf( LoggingSystem::Info, "Opened %s.", name.c_str() );

	configFiles_[name] = configTree;

	return true;
}

bool ConfigSystem::unloadConfig(const std::string& name)
{
	if( !isLoaded( name ) )
	{
		Singleton<LoggingSystem>::instance().writeLogf( LoggingSystem::Warning, "Trying to unload %s when it is not open.", name.c_str() );
		return false;
	}

	Singleton<LoggingSystem>::instance().writeLogf( LoggingSystem::Info, "Unloading %s.", name.c_str());
	configFiles_.erase( configFiles_.find(name) );
	return true;
}

ptree ConfigSystem::getConfigPtree( const std::string& config )
{
	return configFiles_[config];
}
