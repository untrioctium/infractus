#ifndef LIBCOMPUTE_PROGRAM_HPP
#define LIBCOMPUTE_PROGRAM_HPP

#include <boost/property_tree/ptree.hpp>

namespace libcompute
{

/** Class that abstracts running computations on various computational platforms. */
class __attribute__ ((dllexport)) Program
{
public:

	/** Create a blank Program */
	Program(){}

	void setWorkingDirectory( const std::string& dir ) { workingDirectory_ = dir; }
	std::string getWorkingDirectory() { return workingDirectory_; }

	/**
	 * @brief Setup the Program from a XML configuration file.
	 * @param path The path to the configuration file.
	 *
	 * See the \ref XMLFileDoc "XML file documentation" for more information on
	 * how the XML file should be formatted.
	 */
	void load( const std::string& );

	/**
	 * @brief Create and setup a Program from an existing configuration ptree.
	 * @param config The configuration ptree.
	 *
	 * See the \ref XMLFileDoc "XML file documentation" for more information on
	 * how \a config should be structured.
	 */
	Program(const boost::property_tree::ptree & config);
	
	/** Destruct and free up resources. */
	~Program();

	/** Describes the location of storage */
	enum StorageLocation
	{
		Input = 0, ///< Input storage
		Output = 1, ///< Output storage
	};

	/** Describes the location of a program location */
	enum ProgramLocation
	{
		File = 0, ///< The program is located in a file
		Memory = 1, ///< The program is loaded into memory
	};

	/**
	 * @brief Gets a reference to a Parameter.
	 * @param name The name of the Parameter to reference.
	 * @return A reference to the stored Parameter.
	 * @throw UnknownParameterException The Parameter could not be found.
	 */
	Parameter& getParameter( const std::string& name ) throw(UnknownParameterException);

	/** The std::map type used to store the parameters */
	typedef std::map<std::string, Parameter> parameter_storage;

	/** A iterator for the parameter_storage type */
	typedef parameter_storage::iterator iterator;

	/** A const_iterator for the parameter_storage type */
	typedef parameter_storage::const_iterator const_iterator;

	/** Get a iterator pointing to the start of the Parameter table. */
	iterator begin() { return parameters_.begin(); }

	/** Get a iterator pointing to the end of the Parameter table. */
	iterator end() { return parameters_.end(); }

	/** Get a const_iterator pointing to the start of the Parameter table. */
	const_iterator begin() const { return parameters_.begin(); }

	/** Get a const_iterator pointing to the end of the Parameter table. */
	const_iterator end() const { return parameters_.end(); }

	/**
	 * @brief Tests to see if a given name is a valid Parameter name.
	 * @param name The Parameter name to check.
	 * @return True if \a name is a valid Parameter name.
	 */ 
	static bool isValidParameterName( const std::string& name );

	/**
	 * @brief Adds a new Parameter to this Program.
	 * @param name The requested name of the Parameter.
	 * @param type The requested type of the Parameter.
	 * @throw EngineCurrentlyBoundException A Parameter was being added while an engine was bound.
	 * @throw MalformedParameterNameException The \a name argument is not a valid Parameter name.
	 * @throw ParameterAlreadyExistsException The parameter already exists.
	 */
	void addParameter( const std::string& name, Parameter::ParameterType type ) 
		throw( EngineCurrentlyBoundException, MalformedParameterNameException, ParameterAlreadyExistsException );

	/**
	 * @brief Adds a new Parameter array to this Program.
	 * @param name The requested name of the Parameter array.
	 * @param type The requested type of the Parameter array.
	 * @param size The requested size of the Parameter array.
	 * @throw EngineCurrentlyBoundException A Parameter was being added while an engine was bound.
	 * @throw MalformedParameterNameException The \a name argument is not a valid Parameter name.
	 * @throw InvalidSizeException The \a size argument was equal to zero.
	 * @throw ParameterAlreadyExistsException The parameter already exists.
	 */	
	void addParameterArray( const std::string& name, Parameter::ParameterType type, unsigned int size )
		throw( EngineCurrentlyBoundException, MalformedParameterNameException, InvalidSizeException, ParameterAlreadyExistsException );

	/**
	 * @brief Removes a Parameter with the given name.
	 * @param name The name of the Parameter to remove.
	 * @throw EngineCurrentlyBoundException A Parameter was being removed while an engine was bound.
	 * @throw UnknownParameterException The Parameter could not be found.
	 */
	void removeParameter( const std::string& name )
		throw( EngineCurrentlyBoundException, UnknownParameterException );

	/**
	 * @brief Checks to see if a Parameter exists.
	 * @param name The name of the Parameter to check.
	 * @return True if a Parameter named \a name exists.
	 */
	bool parameterExists( const std::string& name ) { return parameters_.find(name) != parameters_.end(); }

	/** Returns a vector containing all current Parameter names. */
	std::vector<std::string> getParameterNames();

	/** Checks to see if this Program has a bound Engine. */
	bool hasBoundEngine() const { return boundEngine_ != NULL; }

	/**
	 * @brief Gets a pointer to the Engine currently bound to this Program.
	 * @throw NoEngineBoundException There is currently no engine bound to this Program.
	 */
	Engine* getBoundEngine() const throw( NoEngineBoundException );

	/**
	 * @brief Binds a Engine to this Program.
	 * @param engine The engine to bind.
	 * @throw EngineCurrentlyBoundException There is already an engine bound to this Program.
	 */
	void bindEngine( Engine* engine ) throw( EngineCurrentlyBoundException );
	
	void unbindEngine() { boundEngine_->unbindProgram(this); boundEngine_ = NULL; }
	
	/**
	 * @brief Checks to see if an engine has a program designated.
	 * @param engine The engine name to check.
	 * @return True if the engine specified by \a name has a program designated.
	 */ 
	bool engineSupported( const std::string& engine ) const { return supportedEngines_.find(engine) != supportedEngines_.end(); }
	
	/**
	 * @brief Gets the program location type for a specified engine.
	 * @param engine The engine name to check for.
	 * @return A ProgramLocation indicating the program's location type.
	 * @throw EngineNotSupportedException The specified engine does not have a program.
	 */
	ProgramLocation getProgramLocationType( const std::string& engine ) const throw( EngineNotSupportedException );
	
	/**
	 * @brief Sets the location of a program for an engine as a file.
	 * @param engine The engine to set for.
	 * @param path The path to the file.
	 *
	 * If there is already a program set for \a engine, it will be replaced with
	 * the one specified.  The file is not checked to exist until the engine tries to
	 * load it.  See the documentation for each engine to find out what kind of file
	 * it expects to find.
	 */
	void setProgramLocationFile( const std::string& engine, const std::string& path );

	/**
	 * @brief Sets the location of a program for an engine as being in memory.
	 * @param engine The engine to set for.
	 * @param program The program in memory.
	 *
	 * If there is already a program set for \a engine, it will be replaced with
	 * the one specified.  See the documentation for each engine to find out what
	 * type it expects \a program to be.
	 */
	void setProgramLocationMemory( const std::string& engine, boost::any program );

	/**
	 * @brief Gets the file path of a program for a engine.
	 * @brief engine The engine to get for.
	 * @return The path to the program.
	 * @throw ProgramLocationTypeMismatchException The program location is in memory.
	 * @throw EngineNotSupportedException The specified engine does not have a program.
	 */
	std::string getProgramLocationFile( const std::string& engine )
		throw( ProgramLocationTypeMismatchException, EngineNotSupportedException );

	/**
	 * @brief Gets the program in memory for a engine.
	 * @brief engine The engine to get for.
	 * @return The program in memory.
	 * @throw ProgramLocationTypeMismatchException The program location is a file.
	 * @throw EngineNotSupportedException The specified engine does not have a program.
	 */
	boost::any getProgramLocationMemory( const std::string& engine )
		throw( ProgramLocationTypeMismatchException, EngineNotSupportedException );

	/**
	 * @brief Gets the program that is used by the bound engine.
	 * @throw NoEngineBoundException There is no engine bound.
	 *
	 * This function is only of interest to Engine developers; a normal end-user
	 * has no need for this function.
	 */
	void* getActiveProgram() throw( NoEngineBoundException );

	/**
	 * @brief Runs this program in the bound engine.
	 * @throw NoEngineBoundException There is currently no engine bound to this Program.
	 */
	void run() throw( NoEngineBoundException );

	/**
	 * @brief Allocates storage for the program through the currently bound engine.
	 * @param width The requested width of the storage.
	 * @param height The requested height of the storage.
	 * @param location The location where to place the allocated storage.
	 * @throw NoEngineBoundException There is currently no engine bound to this Program.
	 * @throw InvalidSizeException The \a width and/or \a height argument was equal to zero.
	 */
	void allocateStorage( unsigned int width, unsigned int height, StorageLocation location, unsigned int index )
		throw( NoEngineBoundException, InvalidSizeException );

	/**
	 * @brief Gets the storage at the specified location.
	 * @param location The location to retreive from.
	 * @return The specified storage.
	 * @throw NoStorageAllocatedException There is no storage in this location.
	 */
	Engine::DataStorage::Ptr getStorage( StorageLocation location, unsigned int index )
		throw( NoStorageAllocatedException );

	/**
	 * @brief Sets the storage at the specified location.
	 * @param location The location to place the storage.
	 * @param storage The storage to set.
	 */
	void setStorage( StorageLocation location, unsigned int index, Engine::DataStorage::Ptr storage );

	unsigned int getStorageCount( StorageLocation location ) { return storageCount_[location]; }

	/**
	 * @brief Swaps the input and output storages.
	 * @throw InputOutputSwapException The types of input and output storages are mismatched.
	 */
	void swapInputOutput( unsigned int index );

	Engine::DataStorage::Info getStorageInfo( StorageLocation location );

	//void setStorageDataType( StorageLocation location, const std::string& type );
	//void getStorageDataType( StorageLocation location, const std::string& type );

private:

	std::string workingDirectory_;

	std::map<std::string, Parameter> parameters_;

	std::map<std::string, std::pair<ProgramLocation, boost::any> > supportedEngines_;
	Engine* boundEngine_;

	void* activeProgram_;

	Engine::DataStorage::Ptr** storage_[2];
	unsigned int storageCount_[2];
	Engine::DataStorage::Info storageDataTypes_[2];
};

};

#endif
