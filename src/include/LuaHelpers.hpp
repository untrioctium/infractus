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

#define ARRAY2D_DEFINE(T)\
	class_<Array2D<T> >("Array2D" #T)\
		.def(constructor<int, int>())\
		.def("size", &Array2D<T>::size)\
		.def("get", &Array2D<T>::get)\
		.def("getPtr", &Array2D<T>::getPtr)\
		.def("getRef", &Array2D<T>::getRef)\
		.def("set", &Array2D<T>::set)\
		.def("array", &Array2D<T>::array)\

#define ARRAY1D_DEFINE_VEC( V_D, V_T ) \
	class_<Array1D<vec##V_D<V_T> > >("Array1Dvec" #V_D #V_T)\
		.def(constructor<int>())\
		.def("size", &Array1D<vec##V_D<V_T> >::size)\
		.def("get", &Array1D<vec##V_D<V_T> >::get)\
		.def("getPtr", &Array1D<vec##V_D<V_T> >::getPtr)\
		.def("getRef", &Array1D<vec##V_D<V_T> >::getRef)\
		.def("set", &Array1D<vec##V_D<V_T> >::set)\
		.def("array", &Array1D<vec##V_D<V_T> >::array)

void translateLibcomputeException(lua_State* L, libcomputeException const& e)
{
	std::string finalError = boost::diagnostic_information(e);
	lua_pushstring(L, finalError.c_str());
}

int handleErrors( lua_State* L )
{
	lua_Debug d;
	lua_getstack(L, 1, &d);
	lua_getinfo(L, "Sln", &d);

	std::string err = lua_tostring(L, -1);
	lua_pop(L, 1);

 std::stringstream msg;
   msg << d.short_src << ":" << d.currentline;

   if (d.name != 0)
   {
      msg << "(" << d.namewhat << " " << d.name << ")";
   }
   msg << " " << err;
   lua_pushstring(L, msg.str().c_str());
   return 1;

}

template<typename T> T parameterGet( Parameter* const parameter )
{
	return T(*parameter);
}

template<typename T> T& parameterGetRef( Parameter* const parameter )
{
	return (T&)(*parameter);
}

template<typename T> void parameterSet( Parameter* const parameter, T value )
{
	(*parameter) = value;
}

Parameter parameterAt( Parameter* const parameter, unsigned int index )
{
	return (*parameter)[index];
}

std::string vectorAt( std::vector<std::string>* const vec, unsigned int index )
{
	return vec->at(index);
}

template<typename T> float matGet( T* const mat, int r, int c )
{
	return (*mat)(r,c);
}

template<typename T> void matSet( T* const mat, int r, int c, float val )
{
	(*mat)(r,c) = val;
}

Engine::DataStorage& getStorageVal( Program* const program, Program::StorageLocation location, unsigned int index )
{
	return *(program->getStorage( location, index ));
}

struct VoidWrapper
{
	VoidWrapper( void* v )
	: value(v) {}
	void* value;
};

template<typename T> VoidWrapper createVoidWrapper( Array1D<T>* const array )
{
	return VoidWrapper( (void*) array->array() );
}

void dataStorageCopyToArray( Engine::DataStorage* const storage, VoidWrapper& value )
{
	storage->toArray( value.value );
}

void dataStorageCopyFromArray( Engine::DataStorage* const storage, VoidWrapper& value )
{
	storage->fromArray( value.value );
}

Texture dataStorageToTexture( Engine::DataStorage* const storage )
{
	Texture texture;
	texture.location = boost::any_cast<GLuint>(storage->getDataStorage());
	Engine::DataStorage::Info info = storage->getInfo();
	texture.w = info.width;
	texture.h = info.height;
	return texture;
}

typedef unsigned char ubyte;

void programSetLocationMemoryString( Program* const program, const std::string& engine, const std::string& data )
{
	program->setProgramLocationMemory( engine, data );
}

Engine::DataStorage::Ptr loadImage( Engine* engine, const std::string& path )
{
	SDL_Surface* bitmap = IMG_Load( path.c_str() );
	SDL_Surface* surface = SDL_DisplayFormatAlpha( bitmap );
	SDL_FreeSurface( bitmap );
	
	Engine::DataStorage::Info info;
	
	info.size = 4;
	info.type = Engine::DataStorage::Byte;
	
	Engine::DataStorage::Ptr storage = engine->allocateStorage( info, surface->w, surface->h );
	storage->fromArray( surface->pixels );
	SDL_FreeSurface(surface);
	return storage;
	
}

Engine::DataStorage::Ptr engineFromTexture( Engine* const engine, Texture texture )
{
	Engine::DataStorage::Info info;
	
	info.size = 4;
	info.type = Engine::DataStorage::Float;
	info.width = texture.w;
	info.height = texture.h;
	
	Engine::DataStorage::Ptr storage = engine->emptyStorage();
	storage->setInfo(info);
	storage->setDataStorage( texture.location );
	return storage;
}

Engine* pluginToEngine( Plugin* const plugin )
{
	return (Engine*) plugin;
}

int luaPrint( lua_State* L )
{
	std::string name = luabind::object_cast<InfractusProgram*>(luabind::globals(L)["self"])->getName();
	//Singleton<Infractus>::instance().printToConsole( name + ": " + std::string(lua_tostring(L, -1)) );
	lua_pop(L, 1);
	return 0;
}

luabind::object ptreeChildren( const luabind::object& obj)
{
	ptree tree = luabind::object_cast<ptree>(obj);

	lua_State* L = obj.interpreter();	
	try
	{
		ptree tree = luabind::object_cast<ptree>(obj);

		luabind::object children = luabind::newtable(L);
		
		int pos = 1;
		BOOST_FOREACH( ptree::value_type child, tree )
			children[pos++] = child;
		
		return children;
	}
	catch(...)
	{
		return luabind::object();
	}
}

ptree& ptreeGetChild( ptree* const tree, const std::string& path )
{
	return tree->get_child(path);
}

std::string ptreeGet( ptree* const tree, const std::string& path )
{
	return tree->get<std::string>(path, std::string());
}

std::string ptreeData( ptree* const tree )
{
	return tree->data();
}
