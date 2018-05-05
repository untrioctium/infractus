#define LIP_VOID_FUNCTION(name) \
	void name() throw(InfractusProgramException)\
	{\
		try\
		{\
			call<void>(#name);\
		}\
		catch( luabind::error& e )\
		{\
			extractErrorAndThrow();\
		}\
	}\
	static void default_##name( InfractusProgram* ptr )\
	{\
		ptr->InfractusProgram::name();\
	}\

struct LuaInfractusProgram: InfractusProgram, luabind::wrap_base
{

	LuaInfractusProgram() : InfractusProgram() {}

	~LuaInfractusProgram() { lua_close(L); }
	
	void init(  bool usingTextureSource, unsigned int width, unsigned int height ) throw(InfractusProgramException)
	{
		try
		{
			call<void>("init", usingTextureSource, width, height);
		}
		catch( luabind::error& e )
		{
			extractErrorAndThrow();
		}
		catch( libcompute::libcomputeException& e )
		{
			BOOST_THROW_EXCEPTION( InfractusProgramException() << 
			InfractusProgramException::errorString(boost::diagnostic_information(e)));
		}
	}
	
	static void default_init( InfractusProgram* ptr, bool usingTextureSource, unsigned int width, unsigned int height ) 
	{
		ptr->InfractusProgram::init(usingTextureSource, width, height);
	}
	
	Texture getOutput() throw(InfractusProgramException)
	{
		try
		{
			return call<Texture>("getOutput");
		}
		catch( luabind::error& e )
		{
			extractErrorAndThrow();
		}
		return Texture();
	}
	
	static Texture default_getOutput( InfractusProgram* ptr )
	{
		return ptr->InfractusProgram::getOutput();
	}
	
	void run( int dt, float scale ) throw(InfractusProgramException)
	{
		try
		{
			call<void>("run", dt, scale);
		}
		catch( luabind::error& e )
		{
			extractErrorAndThrow();
		}
	}
	
	void setUsedTexture( Texture texture ) throw(InfractusProgramException)
	{	
		try
		{
			call<void>("setUsedTexture", texture);
		}
		catch( luabind::error& e )
		{
			extractErrorAndThrow();
		}
	}
	
	static void default_setUsedTexture( InfractusProgram* ptr, Texture texture )
	{
		ptr->InfractusProgram::setUsedTexture(texture);
	}
	
	static void default_run( InfractusProgram* ptr, int dt, float scale )
	{
		ptr->InfractusProgram::run(dt, scale);
	}
	
	LIP_VOID_FUNCTION(input)
	LIP_VOID_FUNCTION(draw)
	
	void setLuaState( lua_State* state ) { L = state; }
	
	std::string executeCommand( const std::string& command ) throw(InfractusProgramException)
	{
		if( luaL_dostring( L, command.c_str() ) == 1 )
			extractErrorAndThrow();
			
		if( lua_isstring( L, -1 ) )
		{
			std::string result = lua_tostring( L, -1 );
			lua_pop(L, 1);
			return result;
		}
		else
			return std::string();
	}
	
private:

	void extractErrorAndThrow() throw(InfractusProgramException)
	{
		std::string error = lua_tostring(L,-1);
		lua_pop(L,1);
		BOOST_THROW_EXCEPTION( InfractusProgramException() << 
			InfractusProgramException::errorString(error));
	}

	lua_State* L;
};
	
	
