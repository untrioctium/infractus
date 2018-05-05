#ifndef LIBCOMPUTE_SHAREDLIBRARY_HPP
#define LIBCOMPUTE_SHAREDLIBRARY_HPP

namespace libcompute
{

class SharedLibraryException
{
public:
	SharedLibraryException(const char* error) : error_(error) { }
	const char* getError() const {return error_.c_str();}

private:
	std::string error_;
};

class SharedLibrary
{
public:
	static SharedLibrary* openSharedLibrary(const std::string& name)
		throw (SharedLibraryException);

	virtual ~SharedLibrary() {}
	
	virtual void* findSymbol(const char* name) = 0;
};

template<class T>
T findSymbol(SharedLibrary& sl, const char* name)
{
	return (T)sl.findSymbol(name);
}

};

#endif
