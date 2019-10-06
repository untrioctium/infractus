#include "Global.hpp"

#include <sol/sol.hpp>

#include <libcompute.hpp>
#include <iostream>
#include "Array.hpp"
#include "GraphicsSystem.hpp"
#include "InputSystem.hpp"
#include "Singleton.hpp"
#include "ConfigSystem.hpp"
#include "LoggingSystem.hpp"
#include "FileSystem.hpp"


#include "InfractusProgram.hpp"
#include "LuaInfractusProgram.hpp"
#include "ProgramLoader.hpp"

#include "Infractus.hpp"

#include <sstream>

using namespace libcompute;

namespace sol {
template<>
struct is_container<boost::property_tree::ptree> : std::false_type {};
}

#include "LuaHelpers.hpp"

class LuaProgramLoader: public ProgramLoader
{
public:
	LuaProgramLoader(): ProgramLoader("LuaProgramLoader") {}
	
	InfractusProgram* createInstance( const std::string& scriptPath )
	{
		auto prog = new LuaInfractusProgram{};
		auto state = new sol::state{};

		loadSolBindings(*state);
		auto def = state->do_file(scriptPath).get<sol::table>();
		def["getWorkingDirectory"] = [prog](sol::table self) { return prog->getWorkingDirectory(); };
		def["setBufferTexture"] = [prog](sol::table self, Texture bufferTexture) { prog->setBufferTexture(bufferTexture);};
		def["getBufferTexture"] = [prog](sol::table self) { return prog->getBufferTexture();};
		prog->setLuaState(state, def);
		return prog;

		/*printf(scriptPath.c_str());
		lua_State* L = luaL_newstate();
		try
		{
			loadLuaBindings(L);
		
			if( luaL_dofile( L, scriptPath.c_str() ) != 0 )
			{
				std::string error = lua_tostring(L, -1);
				lua_close(L);
				std::cout << error << std::endl;
			}
	
			LuaInfractusProgram* instance = luabind::call_function<LuaInfractusProgram*>(L, "getProgram")[ luabind::adopt(luabind::result) ];
			instance->setLuaState(L);
			return instance;
		}
		catch( ...)
		{
			std::string error = lua_tostring(L, -1);
			std::cout << error << std::endl;
			lua_close(L);
		}*/
	}
	
	void loadSolBindings( sol::state& state) {

		state.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::io);

		state["ticks"] = &SDL_GetTicks;
		state["loadImage"] = &loadImage;

#define ARRAY1D_DEFINE(T)\
	class_<Array1D<T> >("Array1D" #T)\
		.def(constructor<int>())\
		.def("size", &Array1D<T>::size)\
		.def("get", &Array1D<T>::get)\
		.def("getPtr", &Array1D<T>::getPtr)\
		.def("getRef", &Array1D<T>::getRef)\
		.def("set", &Array1D<T>::set)\
		.def("setAll", &Array1D<T>::setAll)\
		.def("array", &createVoidWrapper<T>)\

		auto a1f_ut = state.new_usertype<Array1D<float>>("Array1Dfloat", sol::call_constructor, sol::constructors<Array1D<float>(int)>());
		a1f_ut["size"] = &Array1D<float>::size;
		a1f_ut["get"] = &Array1D<float>::get;
		a1f_ut["getPtr"] = &Array1D<float>::getPtr;
		a1f_ut["getRef"] = &Array1D<float>::getRef;
		a1f_ut["set"] = &Array1D<float>::set;
		a1f_ut["setAll"] = &Array1D<float>::setAll;
		a1f_ut["array"] = [](Array1D<float>& self) { return (void*) self.array();};
		
		auto ptree_ut = state.new_usertype<boost::property_tree::ptree>("Ptree", sol::no_constructor);
		ptree_ut["getChild"] = &ptreeGetChild;
		ptree_ut["get"] = &ptreeGet;
		ptree_ut["data"] = &ptreeData;
		ptree_ut["children"] = &ptreeChildren;
		ptree_ut["size"] = &ptree::size;
		ptree_ut["count"] = &ptree::count;

		auto vec2_ut = state.new_usertype<vec2>("vec2", 
			sol::call_constructor, sol::constructors<vec2(), vec2(float, float)>());
		vec2_ut["x"] = &vec2::x;
		vec2_ut["y"] = &vec2::y;
		vec2_ut["arg"] = &vec2::arg;
		vec2_ut["mag"] = &vec2::mag;
		vec2_ut["normalize"] = &vec2::normalize;
		vec2_ut["dot"] = &vec2::dot;
		vec2_ut["comp"] = &vec2::comp;
		vec2_ut["proj"] = &vec2::proj;
		vec2_ut[sol::meta_function::division] = &vec2::operator/;
		vec2_ut[sol::meta_function::multiplication] = &vec2::operator*;
		vec2_ut[sol::meta_function::addition] = &vec2::operator+;
		vec2_ut[sol::meta_function::subtraction] = &vec2::operator-;

		auto vec3_ut = state.new_usertype<vec3>("vec3",
			sol::call_constructor, sol::constructors<vec3(), vec3(float, float, float)>());
		vec3_ut["x"] = &vec3::x;
		vec3_ut["y"] = &vec3::y;
		vec3_ut["z"] = &vec3::z;
		vec3_ut[sol::meta_function::division] = &vec3::operator/;
		vec3_ut[sol::meta_function::multiplication] = &vec3::operator*;
		vec3_ut[sol::meta_function::addition] = &vec3::operator+;
		vec3_ut[sol::meta_function::subtraction] = &vec3::operator-;

		auto vec4_ut = state.new_usertype<vec4>("vec4",
			sol::call_constructor, sol::constructors<vec4(), vec4(float, float, float, float)>());
		vec4_ut["x"] = &vec4::x;
		vec4_ut["y"] = &vec4::y;
		vec4_ut["z"] = &vec4::z;
		vec4_ut["w"] = &vec4::w;
		vec4_ut[sol::meta_function::division] = &vec4::operator/;
		vec4_ut[sol::meta_function::multiplication] = &vec4::operator*;
		vec4_ut[sol::meta_function::addition] = &vec4::operator+;
		vec4_ut[sol::meta_function::subtraction] = &vec4::operator-;

		auto mat3_ut = state.new_usertype<mat3>("mat3",
			sol::call_constructor, sol::constructors<mat3()>());
		mat3_ut["set"] = &matSet<mat3>;
		mat3_ut["get"] = &matGet<mat3>;
		mat3_ut[sol::meta_function::multiplication] = &mat3::operator*;
		mat3_ut[sol::meta_function::addition] = &mat3::operator+;
		mat3_ut[sol::meta_function::subtraction] = &mat3::operator-;

		auto mat4_ut = state.new_usertype<mat4>("mat4",
			sol::call_constructor, sol::constructors<mat4()>());
		mat4_ut["set"] = &matSet<mat4>;
		mat4_ut["get"] = &matGet<mat4>;
		mat4_ut[sol::meta_function::multiplication] = &mat4::operator*;
		mat4_ut[sol::meta_function::addition] = &mat4::operator+;
		mat4_ut[sol::meta_function::subtraction] = &mat4::operator-;

		auto plugin_ut = state.new_usertype<Plugin>("Plugin", sol::no_constructor);
		plugin_ut["toEngine"] = &pluginToEngine;

		auto engine_ut = state.new_usertype<Engine>("Engine", sol::no_constructor,
			sol::base_classes, sol::bases<Plugin>());

		engine_ut["fromTexture"] = &engineFromTexture;
		engine_ut["reduce"] = &Engine::reduce;
		engine_ut["Minimum"] = sol::var(Engine::Minimum);
		engine_ut["Maximum"] = sol::var(Engine::Maximum);
		engine_ut["Sum"] = sol::var(Engine::Sum);

		auto ds_ut = state.new_usertype<Engine::DataStorage>("DataStorage", sol::no_constructor);
		ds_ut["copyToArray"] = &dataStorageCopyToArray;
		ds_ut["copyFromArray"] = &dataStorageCopyFromArray;
		ds_ut["toTexture"] = &dataStorageToTexture;

		auto param_ut = state.new_usertype<Parameter>("Parameter", sol::no_constructor);
		param_ut["getFloat"] = &parameterGet<float>;
		param_ut["setFloat"] = &parameterSet<float>;
		param_ut["getInt"] = &parameterGet<int>;
		param_ut["setInt"] = &parameterSet<int>;
		param_ut["setVec2"] = &parameterSet<vec2>;
		param_ut["getVec2"] = &parameterGetRef<vec2>;
		param_ut["setVec3"] = &parameterSet<vec3>;
		param_ut["getVec3"] = &parameterGetRef<vec3>;
		param_ut["setVec4"] = &parameterSet<vec4>;
		param_ut["getVec4"] = &parameterGetRef<vec4>;
		param_ut["getMat3"] = &parameterGetRef<mat3>;
		param_ut["setMat3"] = &parameterSet<mat3>;
		param_ut["getMat4"] = &parameterGetRef<mat4>;
		param_ut["setMat4"] = &parameterSet<mat4>;
		param_ut["size"] = &Parameter::size;
		param_ut["type"] = &Parameter::type;
		param_ut["at"] = &parameterAt;
		param_ut["Int"] = sol::var(Parameter::Int);
		param_ut["Float"] = sol::var(Parameter::Float);
		param_ut["Vec2"] = sol::var(Parameter::Vec2);
		param_ut["Vec3"] = sol::var(Parameter::Vec3);
		param_ut["Vec4"] = sol::var(Parameter::Vec4);

		auto pm_ut = state.new_usertype<PluginManager>("PluginManager", sol::no_constructor);
		pm_ut["loadPlugin"] = &PluginManager::loadPlugin;
		pm_ut["instance"] = &Singleton<PluginManager>::instance;

		auto prog_ut = state.new_usertype<Program>("Program", sol::call_constructor,
			sol::constructors<Program()>());

		prog_ut["load"] = &Program::load;
		prog_ut["setWorkingDirectory"] = &Program::setWorkingDirectory;
		prog_ut["getStorage"] = &Program::getStorage;
		prog_ut["setStorage"] = &Program::setStorage;
		prog_ut["bindEngine"] = &Program::bindEngine;
		prog_ut["unbindEngine"] = &Program::unbindEngine;
		prog_ut["allocateStorage"] = &Program::allocateStorage;
		prog_ut["run"] = &Program::run;
		prog_ut["addParameter"] = &Program::addParameter;
		prog_ut["addParameterArray"] = &Program::addParameterArray;
		prog_ut["getParameter"] = &Program::getParameter;
		prog_ut["getParameterNames"] = &Program::getParameterNames;
		prog_ut["swapInputOutput"] = &Program::swapInputOutput;
		prog_ut["setProgramLocationFile"] = &Program::setProgramLocationFile;
		prog_ut["setProgramLocationMemoryString"] = &programSetLocationMemoryString;
		prog_ut["getStorageVal"] = &getStorageVal;
		prog_ut["input"] = sol::var(Program::Input);
		prog_ut["output"] = sol::var(Program::Output);

		auto si_ut = state.new_usertype<ScreenInfo>("ScreenInfo", sol::no_constructor);
		si_ut["w"] = sol::readonly(&ScreenInfo::w);
		si_ut["h"] = sol::readonly(&ScreenInfo::h);
		si_ut["d"] = sol::readonly(&ScreenInfo::d);
		si_ut["f"] = sol::readonly(&ScreenInfo::f);

		auto point_ut = state.new_usertype<Point>("Point", sol::call_constructor,
			sol::constructors<Point(float, float), Point(float, float, float)>());
		point_ut["x"] = &Point::x;
		point_ut["y"] = &Point::y;
		point_ut["z"] = &Point::z;

		auto color_ut = state.new_usertype<Color>("Color", sol::call_constructor,
			sol::constructors<Color(float, float, float), Color(float, float, float, float)>());
		color_ut["r"] = &Color::r;
		color_ut["g"] = &Color::g;
		color_ut["b"] = &Color::b;
		color_ut["a"] = &Color::a;

		auto tex_ut = state.new_usertype<Texture>("Texture", sol::call_constructor,
			sol::constructors<Texture()>());
		tex_ut["w"] = sol::readonly(&Texture::w);
		tex_ut["h"] = sol::readonly(&Texture::h);

		auto conf_ut = state.new_usertype<ConfigSystem>("ConfigSystem", sol::no_constructor);
		conf_ut["isLoaded"] = &ConfigSystem::isLoaded;
		conf_ut["loadConfig"] = &ConfigSystem::loadConfig;
		conf_ut["getConfigPtree"] = &ConfigSystem::getConfigPtree;
		conf_ut["instance"] = &Singleton<ConfigSystem>::instance;

		auto gfx_ut = state.new_usertype<GraphicsSystem>("GraphicsSystem", sol::no_constructor);
		gfx_ut["getScreenInfo"] = &GraphicsSystem::getScreenInfo;
		gfx_ut["loadTexture"] = &GraphicsSystem::loadTexture;
		gfx_ut["drawTexture"] = &GraphicsSystem::drawTexture;
		gfx_ut["setDrawColor"] = &GraphicsSystem::setDrawColor;
		gfx_ut["resetDrawColor"] = &GraphicsSystem::resetDrawColor;
		gfx_ut["drawRectangle"] = &GraphicsSystem::drawRectangle;
		gfx_ut["getFontWidth"] = &GraphicsSystem::getFontWidth;
		gfx_ut["getFontHeight"] = &GraphicsSystem::getFontHeight;
		gfx_ut["drawText"] = &GraphicsSystem::drawText;
		gfx_ut["drawLine"] = &GraphicsSystem::drawLine;
		gfx_ut["drawPoint"] = &GraphicsSystem::drawPoint;
		gfx_ut["getFPS"] = &GraphicsSystem::getFPS;
		gfx_ut["delta"] = &GraphicsSystem::delta;
		gfx_ut["setDrawingMode"] = &GraphicsSystem::setDrawingMode;
		gfx_ut["getDrawingMode"] = &GraphicsSystem::getDrawingMode;
		gfx_ut["pushDrawingMode"] = &GraphicsSystem::pushDrawingMode;
		gfx_ut["popDrawingMode"] = &GraphicsSystem::popDrawingMode;
		gfx_ut["pushViewMatrix"] = &GraphicsSystem::pushViewMatrix;
		gfx_ut["popViewMatrix"] = &GraphicsSystem::popViewMatrix;
		gfx_ut["translateView"] = &GraphicsSystem::translateView;
		gfx_ut["rotateView"] = &GraphicsSystem::rotateView;
		gfx_ut["scaleView"] = &GraphicsSystem::scaleView;
		gfx_ut["drawPointArray"] = &GraphicsSystem::drawPointArray;
		gfx_ut["createBufferTexture"] = &GraphicsSystem::createBufferTexture;
		gfx_ut["drawToTexture"] = &GraphicsSystem::drawToTexture;
		gfx_ut["clearToColor"] = &GraphicsSystem::clearToColor;
		gfx_ut["setBlendingMode"] = &GraphicsSystem::setBlendingMode;
		gfx_ut["useTexture"] = &GraphicsSystem::useTexture;
		gfx_ut["setPointSize"] = &GraphicsSystem::setPointSize;
		gfx_ut["saveTexture"] = &GraphicsSystem::saveTexture;
		gfx_ut["freeTexture"] = &GraphicsSystem::freeTexture;
		gfx_ut["instance"] = &Singleton<GraphicsSystem>::instance;
		gfx_ut["TwoD"] = sol::var(GraphicsSystem::TwoD);
		gfx_ut["ThreeD"] = sol::var(GraphicsSystem::ThreeD);
		gfx_ut["Normal"] = sol::var(GraphicsSystem::Normal);
		gfx_ut["Additive"] = sol::var(GraphicsSystem::Additive);

		auto sc_ut = state.new_usertype<ScreenCoordinate>("ScreenCoordinate", sol::no_constructor);
		sc_ut["x"] = sol::readonly(&ScreenCoordinate::x);
		sc_ut["y"] = sol::readonly(&ScreenCoordinate::y);

		auto in_ut = state.new_usertype<InputSystem>("InputSystem", sol::no_constructor);
		in_ut["getKeyState"] = &InputSystem::getKeyState;
		in_ut["getMousePosition"] = &InputSystem::getMousePosition;
		in_ut["getMouseMotion"] = &InputSystem::getMouseMotion;
		in_ut["getMouseButtonState"] = &InputSystem::getMouseButtonState;
		in_ut["getMouseWheelScroll"] = &InputSystem::getMouseWheelScroll;
		in_ut["instance"] = &Singleton<InputSystem>::instance;
		in_ut["K_BACKSPACE"] = sol::var(SDLK_BACKSPACE);
		in_ut["K_TAB"] = sol::var(SDLK_TAB);
		in_ut["K_RETURN"] = sol::var(SDLK_RETURN);
		in_ut["K_ESCAPE"] = sol::var(SDLK_ESCAPE);
		in_ut["K_BACKSPACE"] = sol::var(SDLK_BACKSPACE);
		in_ut["K_TAB"] = sol::var(SDLK_TAB);
		in_ut["K_RETURN"] = sol::var(SDLK_RETURN);
		in_ut["K_PAUSE"] = sol::var(SDLK_PAUSE);
		in_ut["K_ESCAPE"] = sol::var(SDLK_ESCAPE);
		in_ut["K_SPACE"] = sol::var(SDLK_SPACE);
		in_ut["K_COMMA"] = sol::var(SDLK_COMMA);
		in_ut["K_MINUS"] = sol::var(SDLK_MINUS);
		in_ut["K_PERIOD"] = sol::var(SDLK_PERIOD);
		in_ut["K_SLASH"] = sol::var(SDLK_SLASH);
		in_ut["K_0"] = sol::var(SDLK_0);
		in_ut["K_1"] = sol::var(SDLK_1);
		in_ut["K_2"] = sol::var(SDLK_2);
		in_ut["K_3"] = sol::var(SDLK_3);
		in_ut["K_4"] = sol::var(SDLK_4);
		in_ut["K_5"] = sol::var(SDLK_5);
		in_ut["K_6"] = sol::var(SDLK_6);
		in_ut["K_7"] = sol::var(SDLK_7);
		in_ut["K_8"] = sol::var(SDLK_8);
		in_ut["K_9"] = sol::var(SDLK_9);
		in_ut["K_COLON"] = sol::var(SDLK_COLON);
		in_ut["K_SEMICOLON"] = sol::var(SDLK_SEMICOLON);
		in_ut["K_EQUALS"] = sol::var(SDLK_EQUALS);
		in_ut["K_LEFTBRACKET"] = sol::var(SDLK_LEFTBRACKET);
		in_ut["K_BACKSLASH"] = sol::var(SDLK_BACKSLASH);
		in_ut["K_RIGHTBRACKET"] = sol::var(SDLK_RIGHTBRACKET);
		in_ut["K_a"] = sol::var(SDLK_a);
		in_ut["K_b"] = sol::var(SDLK_b);
		in_ut["K_c"] = sol::var(SDLK_c);
		in_ut["K_d"] = sol::var(SDLK_d);
		in_ut["K_e"] = sol::var(SDLK_e);
		in_ut["K_f"] = sol::var(SDLK_f);
		in_ut["K_g"] = sol::var(SDLK_g);
		in_ut["K_h"] = sol::var(SDLK_h);
		in_ut["K_i"] = sol::var(SDLK_i);
		in_ut["K_j"] = sol::var(SDLK_j);
		in_ut["K_k"] = sol::var(SDLK_k);
		in_ut["K_l"] = sol::var(SDLK_l);
		in_ut["K_m"] = sol::var(SDLK_m);
		in_ut["K_n"] = sol::var(SDLK_n);
		in_ut["K_o"] = sol::var(SDLK_o);
		in_ut["K_p"] = sol::var(SDLK_p);
		in_ut["K_q"] = sol::var(SDLK_q);
		in_ut["K_r"] = sol::var(SDLK_r);
		in_ut["K_s"] = sol::var(SDLK_s);
		in_ut["K_t"] = sol::var(SDLK_t);
		in_ut["K_u"] = sol::var(SDLK_u);
		in_ut["K_v"] = sol::var(SDLK_v);
		in_ut["K_w"] = sol::var(SDLK_w);
		in_ut["K_x"] = sol::var(SDLK_x);
		in_ut["K_y"] = sol::var(SDLK_y);
		in_ut["K_z"] = sol::var(SDLK_z);
		in_ut["K_DELETE"] = sol::var(SDLK_DELETE);
		in_ut["K_KP0"] = sol::var(SDLK_KP_0);
		in_ut["K_KP1"] = sol::var(SDLK_KP_1);
		in_ut["K_KP2"] = sol::var(SDLK_KP_2);
		in_ut["K_KP3"] = sol::var(SDLK_KP_3);
		in_ut["K_KP4"] = sol::var(SDLK_KP_4);
		in_ut["K_KP5"] = sol::var(SDLK_KP_5);
		in_ut["K_KP6"] = sol::var(SDLK_KP_6);
		in_ut["K_KP7"] = sol::var(SDLK_KP_7);
		in_ut["K_KP8"] = sol::var(SDLK_KP_8);
		in_ut["K_KP9"] = sol::var(SDLK_KP_9);
		in_ut["K_KP_PERIOD"] = sol::var(SDLK_KP_PERIOD);
		in_ut["K_KP_DIVIDE"] = sol::var(SDLK_KP_DIVIDE);
		in_ut["K_KP_MULTIPLY"] = sol::var(SDLK_KP_MULTIPLY);
		in_ut["K_KP_MINUS"] = sol::var(SDLK_KP_MINUS);
		in_ut["K_KP_PLUS"] = sol::var(SDLK_KP_PLUS);
		in_ut["K_KP_ENTER"] = sol::var(SDLK_KP_ENTER);
		in_ut["K_KP_EQUALS"] = sol::var(SDLK_KP_EQUALS);
		in_ut["K_UP"] = sol::var(SDLK_UP);
		in_ut["K_DOWN"] = sol::var(SDLK_DOWN);
		in_ut["K_RIGHT"] = sol::var(SDLK_RIGHT);
		in_ut["K_LEFT"] = sol::var(SDLK_LEFT);
		in_ut["K_INSERT"] = sol::var(SDLK_INSERT);
		in_ut["K_HOME"] = sol::var(SDLK_HOME);
		in_ut["K_END"] = sol::var(SDLK_END);
		in_ut["K_PAGEUP"] = sol::var(SDLK_PAGEUP);
		in_ut["K_PAGEDOWN"] = sol::var(SDLK_PAGEDOWN);
		in_ut["K_F1"] = sol::var(SDLK_F1);
		in_ut["K_F2"] = sol::var(SDLK_F2);
		in_ut["K_F3"] = sol::var(SDLK_F3);
		in_ut["K_F4"] = sol::var(SDLK_F4);
		in_ut["K_F5"] = sol::var(SDLK_F5);
		in_ut["K_F6"] = sol::var(SDLK_F6);
		in_ut["K_F7"] = sol::var(SDLK_F7);
		in_ut["K_F8"] = sol::var(SDLK_F8);
		in_ut["K_F9"] = sol::var(SDLK_F9);
		in_ut["K_F10"] = sol::var(SDLK_F10);
		in_ut["K_F11"] = sol::var(SDLK_F11);
		in_ut["K_F12"] = sol::var(SDLK_F12);
		in_ut["K_F13"] = sol::var(SDLK_F13);
		in_ut["K_F14"] = sol::var(SDLK_F14);
		in_ut["K_F15"] = sol::var(SDLK_F15);
		in_ut["K_RSHIFT"] = sol::var(SDLK_RSHIFT);
		in_ut["K_LSHIFT"] = sol::var(SDLK_LSHIFT);
		in_ut["K_RCTRL"] = sol::var(SDLK_RCTRL);
		in_ut["K_LCTRL"] = sol::var(SDLK_LCTRL);
		in_ut["K_RALT"] = sol::var(SDLK_RALT);
		in_ut["K_LALT"] = sol::var(SDLK_LALT);
		in_ut["K_LSUPER"] = sol::var(SDLK_OPER);
		in_ut["K_RSUPER"] = sol::var(SDLK_OPER);
		in_ut["K_SYSREQ"] = sol::var(SDLK_SYSREQ);
		in_ut["K_MENU"] = sol::var(SDLK_MENU);
		in_ut["K_POWER"] = sol::var(SDLK_POWER);
		in_ut["up"] = sol::var(InputSystem::Up);
		in_ut["released"] = sol::var(InputSystem::Released);
		in_ut["pressed"] = sol::var(InputSystem::Pressed);
		in_ut["down"] = sol::var(InputSystem::Down);

	}
};
