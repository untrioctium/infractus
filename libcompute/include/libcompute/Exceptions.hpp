#ifndef LIBCOMPUTE_EXCEPTIONS_HPP
#define LIBCOMPUTE_EXCEPTIONS_HPP

#include <boost/exception/all.hpp>

namespace libcompute
{

/** Base exception for all thrown exceptions */
struct libcomputeException: virtual std::exception, virtual boost::exception {};

/** Base exception for Parameter related exceptions **/
struct ParameterException: virtual libcomputeException
{
	/** The Parameter name that raised the exception */
	typedef boost::error_info<struct tagParameterName, std::string> ParameterName;	
};

/** Indicates a non-existent Parameter was requested */
struct UnknownParameterException: virtual ParameterException {};

/** Indicates a malformed Parameter name was requested */
struct MalformedParameterNameException: virtual ParameterException {};

/** Indicates that a parameter already exists */
struct ParameterAlreadyExistsException: virtual ParameterException {};

/** Indicates an invalid Parameter type cast */
struct InvalidParameterCastException: virtual ParameterException
{
	/** The requested from type */
	typedef boost::error_info<struct tagFromType,std::string> FromType;

	/** The requested to type */
	typedef boost::error_info<struct tagToType,std::string> ToType;
};

/** Indicates no engine is bound to a ProgramContext */
struct NoEngineBoundException: virtual libcomputeException {};

/** Indicates an engine is bound to a ProgramContext */
struct EngineCurrentlyBoundException: virtual libcomputeException
{
	typedef boost::error_info<struct tagBoundEngineName,std::string> BoundEngineName;
};

/** Indicates an engine is not supported for a ProgramContext */
struct EngineNotSupportedException: virtual libcomputeException
{
	typedef boost::error_info<struct tagEngineName,std::string> EngineName;
};

/** Indicates the wrong getProgramLocation*() function was called */
struct ProgramLocationTypeMismatchException: virtual libcomputeException {};

/** Indicates a empty storage location was referenced */
struct NoStorageAllocatedException: virtual libcomputeException {};

/** Indicates input and output tried to be swapped though they have different sizes */
struct InputOutputSwapException: virtual libcomputeException {};

/** Indicates an index was selected out of an array's range */
struct IndexOutOfRangeException: virtual libcomputeException 
{
	/** The index that was requested */
	typedef boost::error_info<struct tagRequestedIndex, unsigned int> RequestedIndex;

	/** The size of the array being indexed. */	
	typedef boost::error_info<struct tagArraySize, unsigned int> ArraySize;
};

/** Indicates a string does not represent a valid data type */
struct UnknownTypeException: virtual libcomputeException
{
	/** The type requested */
	typedef boost::error_info<struct tagTypeName, std::string> TypeName;	
};

/** Indicates a "one time only" data setting method was called twice */
struct DataDoubleSetException: virtual libcomputeException {};

/** Indicates uninitialized data was requested */
struct UninitializedDataRequestedException: virtual libcomputeException {};

/** Indicates an invalid size was requested */
struct InvalidSizeException: virtual libcomputeException
{
	/** The name of the argument given the invalid size */
	typedef boost::error_info<struct tagType, std::string> Name;

	/** The size that was requested */	
	typedef boost::error_info<struct tagRequestedSize, unsigned int> RequestedSize;	
};

};
#endif
