#include "libcompute.hpp"
#include <dlfcn.h>
#include <iostream>

namespace libcompute
{

class UnixSharedLibrary : public SharedLibrary
{
public:
	UnixSharedLibrary(const std::string& name);
	~UnixSharedLibrary();

	void* findSymbol(const char* name);

private:
	void* handle_;
};

};

using namespace libcompute;

SharedLibrary* SharedLibrary::openSharedLibrary(const std::string& name)
{
	return new UnixSharedLibrary(name);
}

UnixSharedLibrary::UnixSharedLibrary(const std::string& name)
	: handle_(0)
{
	std::string fullName = "plugins/" + name + "/plugin.so";
	handle_ = dlopen(fullName.c_str(),RTLD_NOW|RTLD_GLOBAL);
	if (handle_ == 0)
	{
		const char* s = dlerror();
		std::cout << s << std::endl;
		throw SharedLibraryException(s?s:"Exact Error Not Reported");
	}
}

UnixSharedLibrary::~UnixSharedLibrary() { dlclose(handle_); }

void* UnixSharedLibrary::findSymbol(const char* name)
{
	void* sym = dlsym(handle_,name);
	if (sym == 0)
{
			const char* s = dlerror();
		std::cout << s << std::endl;
				throw SharedLibraryException("Symbol Not Found");
}

	else
		return sym;
}