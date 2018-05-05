class ProgramLoader: public libcompute::Plugin
{
public:
	ProgramLoader( const char* name ): libcompute::Plugin( name, "ProgramLoader" ) {}
	
	virtual InfractusProgram* createInstance( const std::string& scriptPath ) throw(InfractusProgramException) = 0;
};
