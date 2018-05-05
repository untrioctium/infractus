#include "libcompute.hpp"

#include <fstream>

using namespace libcompute;

std::string Plugin::readFile( const std::string& filePath, bool programRoot ) const
{
	std::string fullPath;

	if( programRoot )
		fullPath = filePath;
	else		
		fullPath = "plugins/" + pluginName_ + "/" + filePath;

	std::ifstream file(fullPath.c_str());

	if( !file.is_open() ) return std::string();

	std::string contents;
	std::string currentLine;
	while( !file.eof() )
	{
		getline( file, currentLine );
		contents += currentLine + "\n";
	}
	file.close();
	return contents;
}

PluginManager::~PluginManager()
{
	//std::pair<std::string, PluginInfo*> v;
	//BOOST_FOREACH( v, pluginMap_ )
	//	delete v.second;
}

Plugin* PluginManager::loadPlugin(const std::string& name)
	throw (SharedLibraryException)
{
	if (pluginMap_.count(name) > 0)
		return (pluginMap_[name])->plugin;

	PluginInfo* pi = new PluginInfo;
	pi->library = SharedLibrary::openSharedLibrary(name);
	plugin_init_func pif = 
		findSymbol<plugin_init_func>(*pi->library,"plugin_init");

	pi->plugin = (*pif)();
	if (!pi->plugin)
	{
		delete pi;
		throw SharedLibraryException("plugin_init error");
	}
	pluginMap_[name]=pi;
	return (pi->plugin);
}
