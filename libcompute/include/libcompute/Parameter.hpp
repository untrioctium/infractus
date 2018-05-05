#ifndef LIBCOMPUTE_PARAMETER_HPP
#define LIBCOMPUTE_PARAMETER_HPP

#include <boost/bimap.hpp>
#include <boost/lexical_cast.hpp>

namespace libcompute
{

class Program;

/** Class that wraps getting and setting parameters in a safe and easy way */
class __attribute__ ((dllexport)) Parameter
{
public:

	Parameter();
	Parameter( const Parameter& other );

	void operator=( const Parameter& other );

	/** All parameter types that are supported. */
	enum ParameterType
	{
		Int, ///< Represents the int data type.
		Float, ///< Represents the float data type.
		Vec2, ///< Represents the vec2 data type
		Vec3, ///< Represents the vec3 data type
		Vec4, ///< Represents the vec4 data type
		Mat3, ///< Represents the mat3 data type
		Mat4
	};

	/**
	 * @brief Gets a ParameterType from an input string.
	 * @param name The string to extract the ParameterType from.
	 * @return The resulting ParameterType.
	 * @throw UnknownTypeException The parameter type name does not match any supported types.
	 */
	static ParameterType typeFromName( const std::string& name ) throw(UnknownTypeException);

	/**
	 * @brief Gets a string representation of a ParameterType
	 * @param type The ParameterType to convert to a string.
	 * @return A string representing the parameter type. 
	 * @throw UnknownTypeException The ParameterType is invalid.
	 */
	static std::string nameFromType( ParameterType type ) throw(UnknownTypeException);

	/** Frees any memory owned by this Parameter */
	~Parameter();

	/**
	 * @brief Allows easy access to the contents of array parameters.
	 * @param index The index to access.
	 * @return A Parameter that references the given index.
	 * @throw IndexOutOfRangeException \a index was outside the array's valid indexes.
	 *
	 * The design of this operator allows a Parameter to be easily access and modified
	 * like it was a "natural" array.  The returned Parameter can be used like any other
	 * Parameter object to manipulate the contained data.  Suppose you had a Parameter
	 * reference named \c float_array, which is naturally an array of floats of some size \a n.
	 * The following code could be used to access or set the contained values:
	 * @code
	 * 	float value = float_array[2]; // value now holds the contents of the second index.
	 *	float_array[1] = 3.14f; // index 1 of float_array is now 3.14.
	 * @endcode
	 * Parameters that are not arrays (i.e. size one) can still be subscripted, but the only
	 * valid \a index will be 0.  It's not reccomended size one parameters are ever accessed this way
	 * because the overhead is larger than just accessing the value directly.
	 */
	Parameter operator[]( unsigned int index ) throw( IndexOutOfRangeException );

	/**
	 * @brief Get the int value of this parameter.
	 * @return An integer that contains the value of the parameter.
	 * @throw InvalidParameterCastException The type of this parameter is not 
	 *                             compatible with a conversion to a int.
	 *
	 * If the type of this Parameter is float, a conversion to int will be made.
	 */
	operator int() throw(InvalidParameterCastException);

	/**
	 * @brief Get the float value of this parameter.
	 * @return A float that contains the value of the parameter.
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a float.
	 *
	 * If the type of this Parameter is int, a conversion to float will be made.
	 */
	operator float() throw(InvalidParameterCastException);

	/**
	 * @brief Get the a reference to the vec2 value of this parameter.
	 * @return A reference to the vec2 value. 
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a vec2.
	 */
	operator vec2&() throw(InvalidParameterCastException);

	/**
	 * @brief Get the a reference to the vec3 value of this parameter.
	 * @return A reference to the vec3 value. 
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a vec3.
	 */
	operator vec3&() throw(InvalidParameterCastException);

	/**
	 * @brief Get the a reference to the vec4 value of this parameter.
	 * @return A reference to the vec4 value. 
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a vec4.
	 */
	operator vec4&() throw(InvalidParameterCastException);
	
	operator mat3&() throw(InvalidParameterCastException);
	operator mat4&() throw(InvalidParameterCastException);
	
	/**
	 * @brief Get a int* to the value of this parameter.
	 * @return A int* pointing to this parameter's value.
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a int*.
	 *
	 * Use this sparingly and with good reason, as no index checking
	 * is performed on straight pointer arrays.
	 */
	operator int*() throw(InvalidParameterCastException);

	/**
	 * @brief Get a float* to the value of this parameter.
	 * @return A float* pointing to this parameter's value.
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a float*.
	 *
	 * Use this sparingly and with good reason, as no index checking
	 * is performed on straight pointer arrays.
	 */
	operator float*() throw(InvalidParameterCastException);

	/**
	 * @brief Get a vec2* to the value of this parameter.
	 * @return A vec2* pointing to this parameter's value.
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a vec2*.
	 *
	 * Use this sparingly and with good reason, as no index checking
	 * is performed on straight pointer arrays.
	 */
	operator vec2*() throw(InvalidParameterCastException);

	/**
	 * @brief Get a vec3* to the value of this parameter.
	 * @return A vec3* pointing to this parameter's value.
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a vec3*.
	 *
	 * Use this sparingly and with good reason, as no index checking
	 * is performed on straight pointer arrays.
	 */
	operator vec3*() throw(InvalidParameterCastException);

	/**
	 * @brief Get a vec4* to the value of this parameter.
	 * @return A vec4* pointing to this parameter's value.
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a vec4*.
	 *
	 * Use this sparingly and with good reason, as no index checking
	 * is performed on straight pointer arrays.
	 */
	operator vec4*() throw(InvalidParameterCastException);

	operator mat3*() throw(InvalidParameterCastException);
	operator mat4*() throw(InvalidParameterCastException);
	
	/**
	 * @brief Syntactic sugar for getting a vec2 reference from this parameter.
	 * @return A reference to the vec2 value. 
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a vec2.
	 *
	 * Use this to clarify code when only accessing or updating one
	 * element of a vec2 value, e.g.:
	 * @code
	 * 	((vec2&)someParameter).x = 5; // ok, but too many parenthesis for my taste
	 * 	someParameter.vec2Ref().x = 5; // a bit more clearer
	 * 	vec2& refOne = someParameter.vec2Ref(); // but this is redundant...
	 *	vec2& refTwo = someParameter; // ...as this is all you need to get a reference
	 * @endcode	
	 */
	vec2& vec2Ref() throw(InvalidParameterCastException) { return (vec2&)(*this); }

	/**
	 * @brief Syntactic sugar for getting a vec3 reference from this parameter.
	 * @return A reference to the vec3 value. 
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a vec3.
	 *
	 * See the vec2Ref() method for additional usage details.
	 */
	vec3& vec3Ref() throw(InvalidParameterCastException) { return (vec3&)(*this); }

	/**
	 * @brief Syntactic sugar for getting a vec4 reference from this parameter.
	 * @return A reference to the vec4 value. 
	 * @throw InvalidParameterCastException The type of this parameter is not
	 *                             compatible with a conversion to a vec4.
	 *
	 * See the vec2Ref() method for additional usage details.
	 */
	vec4& vec4Ref() throw(InvalidParameterCastException) { return (vec4&)(*this); }
	
	mat3& mat3Ref() throw(InvalidParameterCastException) { return (mat3&)(*this); }
	mat4& mat4Ref() throw(InvalidParameterCastException) { return (mat4&)(*this); }

	void operator=( const int& ) throw(InvalidParameterCastException);
	void operator=( const float& ) throw(InvalidParameterCastException);
	void operator=( const vec2& ) throw(InvalidParameterCastException);
	void operator=( const vec3& ) throw(InvalidParameterCastException);
	void operator=( const vec4& ) throw(InvalidParameterCastException);
	void operator=( const mat3& ) throw(InvalidParameterCastException);
	void operator=( const mat4& ) throw(InvalidParameterCastException);
	
	/** Get the type of the Parameter */
	ParameterType type() { return type_; }

	/** Get the size of the Parameter */
	unsigned int size() { return size_; }

	/** Get the name of the Parameter */
	std::string name() { return name_; }
private:

	friend class Program;

	Parameter( const std::string& name, ParameterType type, unsigned int size );
	Parameter( const std::string& name, ParameterType type );

	ParameterType type_;
	void* value_;
	unsigned int size_;
	bool isArrayChild_;
	std::string name_;

	static boost::bimap<std::string, ParameterType> parameterTypeTable_;
	static boost::bimap<std::string, ParameterType> initParameterTypeTable();

	static std::map<ParameterType, unsigned int> sizeTable_;
	static std::map<ParameterType, unsigned int> initSizeTable();

	static void* allocateStorage( ParameterType type, unsigned int size );

};

};
#endif
