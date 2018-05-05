class ProgramManager;

/** 
 * @brief Class that stores information on a program. 
 *
 * This class holds all attributes and information pertaining
 * to a InfractusProgram loaded from the defining program.xml.
 * For more information on the format the file should be in,
 * see \ref InfractusProgramXMLRef "InfractusProgram XML Reference".
 * For more information on possible attributes and how they affect
 * how a program is used, see the \ref AttributeRef "Attributes Reference".
 */
class ProgramInfo
{
public:

	/** Type used for storing info fields. */
	typedef std::map<std::string, std::string> info_type;

	/** Type used for storing attributes. */
	typedef std::set<std::string> attributes_type;

	ProgramInfo() {}

	/**
	 * @brief Gets a single info field.
	 * @param field The field to retrieve.
	 * @return The string data associated with the field if it exists,
	 *         else a blank string.
	 */
	std::string getInfoField( const std::string& field )
	{
		if( info_.count(field) == 0 ) return std::string();
		return info_[field];
	}
	
	/**
	 * @brief Checks to see if a InfractusProgram has an attribute.
	 * @param attributeName The attribute to query.
	 * @return True if the InfractusProgram has an attribute.
	 */
	bool hasAttribute( const std::string& attributeName )
	{
		return attributes_.count(attributeName) > 0;
	}
	
	/** Gets all info fields contained in this Info. */
	info_type getInfo()
	{
		return info_;
	}

	/** Gets all attributes contained in this Info. */
	attributes_type getAttributes()
	{
		return attributes_;
	}

private:
	
	friend class ProgramManager;
	attributes_type attributes_;
	info_type info_;
};

class ProgramManager
{
public:
	ProgramManager( const fs::path& programsDir)
	: programsDir_(programsDir) { loadAllInfo(); }
	
	InfractusProgram* createInstance( const std::string& programName );
	
	std::vector<std::string> getAvaliablePrograms();
	
	ProgramInfo getProgramInfo( const std::string& programName );
	
	bool programExists( const std::string& programName ) { return avaliablePrograms_.count(programName) > 0; }
	
private:

	void loadAllInfo();

	fs::path programsDir_;
	std::map<std::string, ProgramInfo> avaliablePrograms_;
};
	
