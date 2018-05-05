/** Class that manages loading configuration files */
class ConfigSystem
{
public:
	/** Default constructor that does nothing. */
	ConfigSystem() {}

	/** Sets up the configuration system. */
	void init();

	/**
	 * @brief Indicates if a specific configuration file is loaded.
	 * @param name The name of the file.
	 * @return True if the configuration file is currently loaded.
	 */
	bool isLoaded( const std::string& name );

	/**
	 * @brief Loads a configuration file.
	 * @param name The name of the file.
	 * @return True if the configuration file was successfully loaded or was already loaded.
	 */
	bool loadConfig( const std::string& name );

	/**
	 * @brief Unloads a configuration file.
	 * @param name The name of the file.
	 * @return True if the file was unloaded, false if the file wasn't loaded in the first place.
	 */
	bool unloadConfig( const std::string& name );

	/**
	 * @brief Gets a configuration file's property tree.
	 * @name The name of the file.
	 * @return A boost::property_tree::ptree representing the contents
	 *         of the configuration file.
	 */
	ptree getConfigPtree( const std::string& name );

private:

	std::map<std::string, ptree> configFiles_;

};
