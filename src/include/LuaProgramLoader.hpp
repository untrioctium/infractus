#include "Global.hpp"

#include <lua.hpp>
#include <luabind/luabind.hpp>
#include <luabind/class.hpp>
#include <luabind/exception_handler.hpp>
#include <luabind/operator.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/adopt_policy.hpp>

#include <boost/exception/diagnostic_information.hpp> 
#include <boost/exception_ptr.hpp> 
#include <libcompute.hpp>

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

#include <boost/cast.hpp>

#include <sstream>

using namespace libcompute;
#include "LuaHelpers.hpp"

class LuaProgramLoader: public ProgramLoader
{
public:
	LuaProgramLoader(): ProgramLoader("LuaProgramLoader") {}
	
	InfractusProgram* createInstance( const std::string& scriptPath ) throw(InfractusProgramException)
	{
		printf(scriptPath.c_str());
		lua_State* L = luaL_newstate();
		try
		{
			loadLuaBindings(L);
		
			if( luaL_dofile( L, scriptPath.c_str() ) != 0 )
			{
				std::string error = lua_tostring(L, -1);
				lua_close(L);
				BOOST_THROW_EXCEPTION( InfractusProgramException() << 
					InfractusProgramException::errorString(error));
			}
	
			LuaInfractusProgram* instance = luabind::call_function<LuaInfractusProgram*>(L, "getProgram")[ luabind::adopt(luabind::result) ];
			instance->setLuaState(L);
			return instance;
		}
		catch( luabind::error& e )
		{
			std::string error = lua_tostring(L, -1);
			lua_close(L);
			BOOST_THROW_EXCEPTION( InfractusProgramException() << 
				InfractusProgramException::errorString(error));
		}
	}
	
	void loadLuaBindings( lua_State* L )
	{
		luaL_openlibs(L);

		using namespace luabind;

		open(L);

		luabind::register_exception_handler<libcomputeException>(&translateLibcomputeException);
		luabind::set_pcall_callback( &handleErrors );

		lua_pushcfunction( L, luaPrint );
		lua_setglobal(L, "print");

		module(L)
		[	
			def("ticks", &SDL_GetTicks), 
			//def("test", &test),
			ARRAY1D_DEFINE(ubyte),
			ARRAY1D_DEFINE(float),
			def("loadImage", &loadImage),
			//ARRAY1D_DEFINE_VEC( 4, float ),
			class_<ptree::value_type>("PtreeValue")
				.def_readonly("first", &ptree::value_type::first)
				.def_readonly("second", &ptree::value_type::second),
			class_<ptree>("Ptree")
				.def("getChild", &ptreeGetChild)
				.def("get", &ptreeGet)
				.def("data", &ptreeData)
				.def("children", &ptreeChildren)
				.def("size", &ptree::size)
				.def("count", &ptree::count),
			class_<vec2>("vec2")
				//.def(tostring(self))
				.def(constructor<>())
				.def(constructor<float,float>())
				.def_readwrite("x", &vec2::x)
				.def_readwrite("y", &vec2::y)
				.def("arg", &vec2::arg)
				.def("mag", &vec2::mag)
				.def("normalize", &vec2::normalize)
				.def("dot", &vec2::dot)
				.def("comp", &vec2::comp)
				.def("proj", &vec2::proj)
				.def(const_self/float())
				.def(const_self*float())
				.def(const_self + vec2())
				.def(const_self - vec2()),
			class_<vec3>("vec3")
				.def(constructor<>())
				.def(constructor<float, float, float>())
				.def_readwrite("x", &vec3::x)
				.def_readwrite("y", &vec3::y)
				.def_readwrite("z", &vec3::z)
				.def(const_self/float())
				.def(const_self*float())
				.def(const_self + vec3())
				.def(const_self - vec3()),
			class_<vec4>("vec4")
				.def(constructor<>())
				.def(constructor<float,float,float,float>())
				.def_readwrite("x", &vec4::x)
				.def_readwrite("y", &vec4::y)
				.def_readwrite("z", &vec4::z)
				.def_readwrite("w", &vec4::w)
				.def(const_self/float())
				.def(const_self*float())
				.def(const_self + vec4())
				.def(const_self - vec4()),
			class_<mat3>("mat3")
				.def(constructor<>())
				.def("set", &matSet<mat3>)
				.def("get", &matGet<mat3>)
				.def(self * float())
				.def(self + other<mat3>())
				.def(self - other<mat3>()),
			class_<mat4>("mat4")
				.def(constructor<>())
				.def("set", &matSet<mat4>)
				.def("get", &matGet<mat4>)
				.def(self * float())
				.def(self + other<mat4>())
				.def(self - other<mat4>()),
			class_<Plugin>("Plugin")
				.def("toEngine", &pluginToEngine),
			class_<Engine, Plugin>("Engine")
				.def("fromTexture", &engineFromTexture)
				.def("reduce", &Engine::reduce)
				.enum_("reduction_types")
				[
					value("Minimum", Engine::Minimum),
					value("Maximum", Engine::Maximum),
					value("Sum", Engine::Sum)
				],
			class_<Engine::DataStorage>("DataStorage")
				.def("copyToArray", &dataStorageCopyToArray)
				.def("copyFromArray", &dataStorageCopyFromArray)
				.def("toTexture", &dataStorageToTexture),
			class_<Engine::DataStorage::Ptr>("Engine_DataStorage_Ptr"),
			class_<VoidWrapper>("VoidWrapper"),
			class_<Parameter>("Parameter")
				.def("getFloat", &parameterGet<float>)
				.def("setFloat", &parameterSet<float>)
				.def("getInt", &parameterGet<int>)
				.def("setInt", &parameterSet<int>)
				.def("setVec2", &parameterSet<vec2>)
				.def("getVec2", &parameterGetRef<vec2>)
				.def("setVec3", &parameterSet<vec3>)
				.def("getVec3", &parameterGetRef<vec3>)
				.def("setVec4", &parameterSet<vec4>)
				.def("getVec4", &parameterGetRef<vec4>)
				.def("getMat3", &parameterGetRef<mat3>)
				.def("setMat3", &parameterSet<mat3>)
				.def("getMat4", &parameterGetRef<mat4>)
				.def("setMat4", &parameterSet<mat4>)
				.def("size", &Parameter::size)
				.def("type", &Parameter::type)
				.def("at", &parameterAt)
				.enum_("parameter_types")
				[
					value("Int", Parameter::Int),
					value("Float", Parameter::Float),
					value("Vec2", Parameter::Vec2),
					value("Vec3", Parameter::Vec3),
					value("Vec4", Parameter::Vec4),
					value("Mat3", Parameter::Mat3),
					value("Mat4", Parameter::Mat4)
				],
			class_<PluginManager>("PluginManager")
				.def("loadPlugin", &PluginManager::loadPlugin)
				.scope
				[
					def("instance", &Singleton<PluginManager>::instance)
				],
			class_<InfractusProgram, LuaInfractusProgram>("InfractusProgram")
				.def(constructor<>())
				.def("init", &InfractusProgram::init, &LuaInfractusProgram::default_init)
				.def("input", &InfractusProgram::input, &LuaInfractusProgram::default_input)
				.def("run", &InfractusProgram::run, &LuaInfractusProgram::default_run)
				.def("draw", &InfractusProgram::draw, &LuaInfractusProgram::default_draw)
				.def("getOutput", &InfractusProgram::getOutput, &LuaInfractusProgram::default_getOutput)
				.def("getBufferTexture", &InfractusProgram::getBufferTexture)
				.def("setBufferTexture", &InfractusProgram::setBufferTexture)
				.def("getWorkingDirectory", &InfractusProgram::getWorkingDirectory)
				.enum_("ProgramModes")
				[
					value("Static", InfractusProgram::Static),
					value("Dynamic", InfractusProgram::Dynamic),
					value("Playback", InfractusProgram::Playback),
					value("Interactive", InfractusProgram::Interactive)
				],
			class_<Program>("Program")
				.def(constructor<>())
				//.def(constructor<const std::string&>())
				.def("load", &Program::load)
				.def("setWorkingDirectory", &Program::setWorkingDirectory)
				.def("getStorage", &Program::getStorage)
				.def("setStorage", &Program::setStorage)
				.def("bindEngine", &Program::bindEngine)
				.def("unbindEngine", &Program::unbindEngine)
				.def("allocateStorage", &Program::allocateStorage)
				.def("run", &Program::run)
				.def("addParameter", &Program::addParameter)
				.def("addParameterArray", &Program::addParameterArray)
				.def("getParameter", &Program::getParameter)
				.def("getParameterNames", &Program::getParameterNames)
				.def("swapInputOutput", &Program::swapInputOutput)
				.def("setProgramLocationFile", &Program::setProgramLocationFile)
				.def("setProgramLocationMemoryString", &programSetLocationMemoryString)
				.def("getStorageVal", &getStorageVal)
				.enum_("storage_types")
				[
					value("input", Program::Input),
					value("output", Program::Output)
				],
			class_<std::vector<std::string> >("StringVector")
				.def("size", &std::vector<std::string>::size)
				.def("at", &vectorAt),
			class_<ScreenInfo>("ScreenInfo")
				.def_readonly("w", &ScreenInfo::w)
				.def_readonly("h", &ScreenInfo::h)
				.def_readonly("d", &ScreenInfo::d)
				.def_readonly("f", &ScreenInfo::f),
			class_<Point>("Point")
				.def(constructor<float, float>())
				.def(constructor<float, float, float>())
				.def_readwrite("x", &Point::x)
				.def_readwrite("y", &Point::y)
				.def_readwrite("z", &Point::z),
			class_<Color>("Color")
				.def(constructor<float, float, float>())
				.def(constructor<float, float, float, float>())
				.def_readwrite("r", &Color::r)
				.def_readwrite("g", &Color::g)
				.def_readwrite("b", &Color::b)
				.def_readwrite("a", &Color::a),
			class_<Texture>("Texture")
				.def(constructor<>())
				.def_readonly("w", &Texture::w)
				.def_readonly("h", &Texture::h),
			class_<ConfigSystem>("ConfigSystem")
				.def("isLoaded", &ConfigSystem::isLoaded)
				.def("loadConfig", &ConfigSystem::loadConfig)
				.def("getConfigPtree", &ConfigSystem::getConfigPtree)
				.scope
				[
					def("instance", &Singleton<ConfigSystem>::instance)
				],
			class_<GraphicsSystem>("GraphicsSystem")
				.def("getScreenInfo", &GraphicsSystem::getScreenInfo)
				.def("loadTexture", &GraphicsSystem::loadTexture)
				.def("drawTexture", &GraphicsSystem::drawTexture)
				.def("setDrawColor", &GraphicsSystem::setDrawColor)
				.def("resetDrawColor", &GraphicsSystem::resetDrawColor)
				.def("drawRectangle", &GraphicsSystem::drawRectangle)
				.def("getFontWidth", &GraphicsSystem::getFontWidth)
				.def("getFontHeight", &GraphicsSystem::getFontHeight)
				.def("drawText", &GraphicsSystem::drawText)
				.def("drawLine", &GraphicsSystem::drawLine)
				.def("drawPoint", &GraphicsSystem::drawPoint)
				.def("getFPS", &GraphicsSystem::getFPS)
				.def("delta", &GraphicsSystem::delta)
				.def("setDrawingMode", &GraphicsSystem::setDrawingMode)
				.def("getDrawingMode", &GraphicsSystem::getDrawingMode)
				.def("pushDrawingMode", &GraphicsSystem::pushDrawingMode)
				.def("popDrawingMode", &GraphicsSystem::popDrawingMode)
				.def("pushViewMatrix", &GraphicsSystem::pushViewMatrix)
				.def("popViewMatrix", &GraphicsSystem::popViewMatrix)
				.def("translateView", &GraphicsSystem::translateView)
				.def("rotateView", &GraphicsSystem::rotateView)
				.def("scaleView", &GraphicsSystem::scaleView)
				.def("drawPointArray", &GraphicsSystem::drawPointArray)
				.def("createBufferTexture", &GraphicsSystem::createBufferTexture)
				.def("drawToTexture", &GraphicsSystem::drawToTexture)
				.def("clearToColor", &GraphicsSystem::clearToColor)
				.def("setBlendingMode", &GraphicsSystem::setBlendingMode)
				.def("useTexture", &GraphicsSystem::useTexture)
				.def("setPointSize", &GraphicsSystem::setPointSize)
				.def("saveTexture", &GraphicsSystem::saveTexture)
				.def("freeTexture", &GraphicsSystem::freeTexture)
				.enum_("draw_mode")
				[
					value("TwoD", GraphicsSystem::TwoD),
					value("ThreeD", GraphicsSystem::ThreeD)
				]
				.enum_("blending_mode")
				[
					value("Normal", GraphicsSystem::Normal),
					value("Additive", GraphicsSystem::Additive)
				]
				.scope
				[
					def("instance", &Singleton<GraphicsSystem>::instance)
				],
			class_<ScreenCoordinate>("ScreenCoordinate")
				.def_readonly("x", &ScreenCoordinate::x)
				.def_readonly("y", &ScreenCoordinate::y),
			class_<InputSystem>("InputSystem")
				.def("getKeyState", &InputSystem::getKeyState)
				.def("getMousePosition", &InputSystem::getMousePosition)
				.def("getMouseMotion", &InputSystem::getMouseMotion)
				.def("getMouseButtonState", &InputSystem::getMouseButtonState)
				.def("getMouseWheelScroll", &InputSystem::getMouseWheelScroll)
				.enum_("sdl_keys")
				[
					value("K_BACKSPACE",SDLK_BACKSPACE),
					value("K_TAB",SDLK_TAB),
					value("K_RETURN",SDLK_RETURN),
					value("K_ESCAPE",SDLK_ESCAPE),
					value("K_BACKSPACE",SDLK_BACKSPACE),
					value("K_TAB",SDLK_TAB),
					value("K_RETURN",SDLK_RETURN),
					value("K_PAUSE",SDLK_PAUSE),
					value("K_ESCAPE",SDLK_ESCAPE),
					value("K_SPACE",SDLK_SPACE),
					value("K_COMMA",SDLK_COMMA),
					value("K_MINUS",SDLK_MINUS),
					value("K_PERIOD",SDLK_PERIOD),
					value("K_SLASH",SDLK_SLASH),
					value("K_0",SDLK_0),
					value("K_1",SDLK_1),
					value("K_2",SDLK_2),
					value("K_3",SDLK_3),
					value("K_4",SDLK_4),
					value("K_5",SDLK_5),
					value("K_6",SDLK_6),
					value("K_7",SDLK_7),
					value("K_8",SDLK_8),
					value("K_9",SDLK_9),
					value("K_COLON",SDLK_COLON),
					value("K_SEMICOLON",SDLK_SEMICOLON),
					value("K_EQUALS",SDLK_EQUALS),
					value("K_LEFTBRACKET",SDLK_LEFTBRACKET),
					value("K_BACKSLASH",SDLK_BACKSLASH),
					value("K_RIGHTBRACKET",SDLK_RIGHTBRACKET),
					value("K_a",SDLK_a),
					value("K_b",SDLK_b),
					value("K_c",SDLK_c),
					value("K_d",SDLK_d),
					value("K_e",SDLK_e),
					value("K_f",SDLK_f),
					value("K_g",SDLK_g),
					value("K_h",SDLK_h),
					value("K_i",SDLK_i),
					value("K_j",SDLK_j),
					value("K_k",SDLK_k),
					value("K_l",SDLK_l),
					value("K_m",SDLK_m),
					value("K_n",SDLK_n),
					value("K_o",SDLK_o),
					value("K_p",SDLK_p),
					value("K_q",SDLK_q),
					value("K_r",SDLK_r),
					value("K_s",SDLK_s),
					value("K_t",SDLK_t),
					value("K_u",SDLK_u),
					value("K_v",SDLK_v),
					value("K_w",SDLK_w),
					value("K_x",SDLK_x),
					value("K_y",SDLK_y),
					value("K_z",SDLK_z),
					value("K_DELETE",SDLK_DELETE),
					value("K_KP0",SDLK_KP0),
					value("K_KP1",SDLK_KP1),
					value("K_KP2",SDLK_KP2),
					value("K_KP3",SDLK_KP3),
					value("K_KP4",SDLK_KP4),
					value("K_KP5",SDLK_KP5),
					value("K_KP6",SDLK_KP6),
					value("K_KP7",SDLK_KP7),
					value("K_KP8",SDLK_KP8),
					value("K_KP9",SDLK_KP9),
					value("K_KP_PERIOD",SDLK_KP_PERIOD),
					value("K_KP_DIVIDE",SDLK_KP_DIVIDE),
					value("K_KP_MULTIPLY",SDLK_KP_MULTIPLY),
					value("K_KP_MINUS",SDLK_KP_MINUS),
					value("K_KP_PLUS",SDLK_KP_PLUS),
					value("K_KP_ENTER",SDLK_KP_ENTER),
					value("K_KP_EQUALS",SDLK_KP_EQUALS),
					value("K_UP",SDLK_UP),
					value("K_DOWN",SDLK_DOWN),
					value("K_RIGHT",SDLK_RIGHT),
					value("K_LEFT",SDLK_LEFT),
					value("K_INSERT",SDLK_INSERT),
					value("K_HOME",SDLK_HOME),
					value("K_END",SDLK_END),
					value("K_PAGEUP",SDLK_PAGEUP),
					value("K_PAGEDOWN",SDLK_PAGEDOWN),
					value("K_F1",SDLK_F1),
					value("K_F2",SDLK_F2),
					value("K_F3",SDLK_F3),
					value("K_F4",SDLK_F4),
					value("K_F5",SDLK_F5),
					value("K_F6",SDLK_F6),
					value("K_F7",SDLK_F7),
					value("K_F8",SDLK_F8),
					value("K_F9",SDLK_F9),
					value("K_F10",SDLK_F10),
					value("K_F11",SDLK_F11),
					value("K_F12",SDLK_F12),
					value("K_F13",SDLK_F13),
					value("K_F14",SDLK_F14),
					value("K_F15",SDLK_F15),
					value("K_NUMLOCK",SDLK_NUMLOCK),
					value("K_SCROLLOCK",SDLK_SCROLLOCK),
					value("K_RSHIFT",SDLK_RSHIFT),
					value("K_LSHIFT",SDLK_LSHIFT),
					value("K_RCTRL",SDLK_RCTRL),
					value("K_LCTRL",SDLK_LCTRL),
					value("K_RALT",SDLK_RALT),
					value("K_LALT",SDLK_LALT),
					value("K_LSUPER",SDLK_LSUPER),
					value("K_RSUPER",SDLK_RSUPER),
					value("K_SYSREQ",SDLK_SYSREQ),
					value("K_MENU",SDLK_MENU),
					value("K_POWER",SDLK_POWER)
				]
				.enum_("key_states")
				[
					value("up", InputSystem::Up),
					value("released", InputSystem::Released),
					value("pressed", InputSystem::Pressed),
					value("down", InputSystem::Down)
				]
				.scope
				[
					def("instance", &Singleton<InputSystem>::instance)
				]
		];
	}
	
};
