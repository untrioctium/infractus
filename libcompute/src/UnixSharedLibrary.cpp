#include "libcompute.hpp"
#include <windows.h>

namespace libcompute
{

class UnixSharedLibrary : public SharedLibrary
{
public:
	UnixSharedLibrary(const std::string& name)
		throw (SharedLibraryException);
	~UnixSharedLibrary();

	void* findSymbol(const char* name)
		throw (SharedLibraryException);

private:
	HINSTANCE handle_;
};

};

using namespace libcompute;

SharedLibrary* SharedLibrary::openSharedLibrary(const std::string& name)
	throw (SharedLibraryException)
{
	return new UnixSharedLibrary(name);
}

UnixSharedLibrary::UnixSharedLibrary(const std::string& name)
	throw (SharedLibraryException)
	: handle_(0)
{
	UINT emode = SetErrorMode(SEM_FAILCRITICALERRORS);
	std::string fullName = "plugins\\" + name + "\\plugin.dll";
	handle_ = LoadLibrary(fullName.c_str());
	SetErrorMode(emode);
	if (handle_ == 0)
	{
		//const char* s = dlerror();
		throw SharedLibraryException("Exact Error Not Reported");
	}
}

UnixSharedLibrary::~UnixSharedLibrary() { FreeLibrary(handle_); }

void* UnixSharedLibrary::findSymbol(const char* name)
	throw (SharedLibraryException)
{
	void* sym = (void*) GetProcAddress(handle_,name);
	if (sym == 0)
		throw SharedLibraryException("Symbol Not Found");
	else
		return sym;
}
