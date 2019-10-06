struct LuaInfractusProgram: InfractusProgram
{

	LuaInfractusProgram() : InfractusProgram() {}

	~LuaInfractusProgram() { delete state; }
	
	void init(  bool usingTextureSource, unsigned int width, unsigned int height )
	{
		sol::protected_function f = def["init"];
		auto result = f(def, usingTextureSource, width, height);
		if(!result.valid()) {
			sol::error err = result;
			std::cout << err.what() << std::endl;
			throw err;
		}
	}
	
	void run( int dt, float scale )
	{
		sol::protected_function f = def["run"];
		auto result = f(def, dt, scale);
		if(!result.valid()) {
			sol::error err = result;
			std::cout << err.what() << std::endl;
			std::terminate();
		}
	}
	
	void input() { 
		sol::protected_function f = def["input"];
		auto result = f(def);
		if(!result.valid()) {
			sol::error err = result;
			std::cout << err.what() << std::endl;
			std::terminate();
		}
	}
	void draw() { 
		sol::protected_function f = def["draw"];
		auto result = f(def);
		if(!result.valid()) {
			sol::error err = result;
			std::cout << err.what() << std::endl;
			std::terminate();
		}
	}

	Texture getOutput() {
		if( def["getOutput"].valid()) {
			sol::protected_function f = def["getOutput"];
			auto result = f(def);
			if(!result.valid()) {
				sol::error err = result;
				std::cout << err.what() << std::endl;
				std::terminate();
			}
			return result.get<Texture>();
		} else return InfractusProgram::getOutput();
	}
	
	void setLuaState( sol::state* nstate, sol::table ndef ) { state = nstate; def = ndef; }
	
	std::string executeCommand( const std::string& command )
	{
		lua_State* L = state->lua_state();

		if( luaL_dostring( L, command.c_str() ) == 1 ) {
			std::string error = lua_tostring(L,-1);
			lua_pop(L,1);
			std::cout << error << std::endl;
		}

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

	sol::table def;
	sol::state* state;
};
	
	
