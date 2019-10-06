/**
 * @file Plugin.hpp
 * @brief Contains classes and functions needed for loading plugins.
 */

#ifndef LIBCOMPUTE_PLUGIN_HPP
#define LIBCOMPUTE_PLUGIN_HPP

#ifdef _WIN32
#define DLLEXPORT _declspec(dllexport)
#else

#define DLLEXPORT
#endif

namespace libcompute
{

/** Base class for all dynamically loaded plugins. */
class DLLEXPORT Plugin
{
public:

	/**
	 * @brief Sets up the name and type for a plugin.
	 * @param name The name of the plugin.
	 * @param type The type of the plugin.
	 */
	Plugin(const std::string& name, const std::string& type)
	: pluginName_(name)
	, pluginType_(type) {}

	/**
	 * @brief Gets the plugin's name.
	 * @return The plugin's name.
	 */
	std::string pluginName() const { return pluginName_.c_str(); }

	/**
	 * @brief Gets the plugin's type.
	 * @return The plugin's type.
	 */
	std::string pluginType() const { return pluginType_.c_str(); }

	/**
	 * @brief Helper function for plugins that reads a file in a directory.
	 * @param filePath The path to the file to read.
	 * @param fullPath If true, \a filePath will be interpreted as starting from
	 *                 the application's root directory.  If false, \a filePath
	 *                 will be interpreted as starting from the plugin's directory.
	 * @return A std::string containing the file's contents if the file could be
	 *         read.  If the file does not exist or could not be read, a empty
	 *	   std::string is returned.
	 */ 
	std::string readFile( const std::string& filePath, bool fullPath = false ) const;

	virtual ~Plugin() {};

private:

	std::string pluginName_;
	std::string pluginType_;
};

/**
 * @brief Initialization function needed in all plugin files.
 * @return A pointer to a class that inherits from Plugin.
 */
typedef Plugin* (*plugin_init_func)(void);

/** Class that manages loading plugins */
class DLLEXPORT PluginManager
{
public:
	/** Constructor that sets the plugin search path to be the application's root */
	PluginManager()
	: pluginSearchPath_(".") {}

	/** Frees all loaded plugins. */
	~PluginManager();

	/**
	 * @brief Sets the path where plugins will be searched for.
         * @param path The path that will be searched.
	 */
	void setPluginSearchPath( const std::string& path )
	{
		pluginSearchPath_ = path;
	}

	/**
	 * @brief Gets the path plugins are searched for.
	 * @return The path plugins are searched for.
	 */
	std::string getPluginSearchPath()
	{
		return pluginSearchPath_;
	}

	/**
	 * @brief Gets all plugins that are avaliable in the
	 *        plugin search path;
	 * @return A std::vector<std::string> containing all
	 *         avaliable plugins.
	 */
	std::vector<std::string> listAvaliablePlugins();

	/**
	 * @brief Loads a plugin with the given name.
	 * @param name The name of the plugin.
	 * @return A pointer to the loaded plugin.
	 * @throw SharedLibraryException The plugin could not be loaded.
	 */
	Plugin* loadPlugin(const std::string& name);

private:

	struct PluginInfo 
	{
		SharedLibrary* library;
		std::string libraryName;
		Plugin* plugin;

		~PluginInfo() { delete plugin; delete library; }
		PluginInfo() : library(0), plugin(0) {}
	};

	std::map<std::string,PluginInfo* > pluginMap_;
	std::string pluginSearchPath_;
};

};

#endif
