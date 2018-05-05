#include "libcompute.hpp"

using namespace libcompute;

std::map<Parameter::ParameterType, unsigned int> Parameter::sizeTable_ = Parameter::initSizeTable();
boost::bimap<std::string, Parameter::ParameterType> Parameter::parameterTypeTable_ = Parameter::initParameterTypeTable();

boost::bimap<std::string, Parameter::ParameterType> Parameter::initParameterTypeTable()
{
	boost::bimap<std::string, Parameter::ParameterType> table;
	table.insert( boost::bimap<std::string, ParameterType>::value_type("int", Parameter::Int));
	table.insert( boost::bimap<std::string, ParameterType>::value_type("float", Parameter::Float));
	table.insert( boost::bimap<std::string, ParameterType>::value_type("vec2", Parameter::Vec2));	
	table.insert( boost::bimap<std::string, ParameterType>::value_type("vec3", Parameter::Vec3));
	table.insert( boost::bimap<std::string, ParameterType>::value_type("vec4", Parameter::Vec4));
	table.insert( boost::bimap<std::string, ParameterType>::value_type("mat3", Parameter::Mat3));
	table.insert( boost::bimap<std::string, ParameterType>::value_type("mat4", Parameter::Mat4));
	return table;
}

std::map<Parameter::ParameterType, unsigned int> Parameter::initSizeTable()
{
	std::map<Parameter::ParameterType, unsigned int> table;
	table[Parameter::Int] = sizeof(int);
	table[Parameter::Float] = sizeof(float);
	table[Parameter::Vec2] = sizeof(vec2);
	table[Parameter::Vec3] = sizeof(vec3);
	table[Parameter::Vec4] = sizeof(vec4);
	table[Parameter::Mat3] = sizeof(mat3);
	table[Parameter::Mat4] = sizeof(mat4);
	return table;
}

void* Parameter::allocateStorage( Parameter::ParameterType type, unsigned int size )
{
	return malloc( sizeTable_[type] * size );		
}

Parameter::ParameterType Parameter::typeFromName( const std::string& name ) throw(UnknownTypeException)
{
	if( parameterTypeTable_.left.find( name ) == parameterTypeTable_.left.end() )
		BOOST_THROW_EXCEPTION(UnknownTypeException() << UnknownTypeException::TypeName(name));
	else return parameterTypeTable_.left.at(name);
}

std::string Parameter::nameFromType( ParameterType type ) throw(UnknownTypeException)
{
	if( parameterTypeTable_.right.find( type ) == parameterTypeTable_.right.end() )
		BOOST_THROW_EXCEPTION(UnknownTypeException());
	else return parameterTypeTable_.right.at(type);		
}

Parameter::Parameter()
{
	size_ = 0;
	this->isArrayChild_ = false;
}

Parameter::Parameter( const std::string& name, ParameterType type, unsigned int size )
{
	this->size_ = size;
	this->type_ = type;
	this->name_ = name;
	this->value_ = allocateStorage( type_, size_ );
	this->isArrayChild_ = false;
}

Parameter::Parameter( const std::string& name, ParameterType type )
{
	size_ = 1;
	type_ = type;
	value_ = allocateStorage( type_, size_ );
	this->isArrayChild_ = false;
	this->name_ = name;
}

Parameter::~Parameter()
{
	if( size_ != 0 && !isArrayChild_ )
		free( value_ );
}

Parameter::Parameter( const Parameter& other )
{
	this->type_ = other.type_;
	this->value_ = other.value_;
	this->size_ = other.size_;
	this->name_ = other.name_;
	this->isArrayChild_ = other.isArrayChild_;
	if( isArrayChild_ )
		this->value_ = other.value_;
	else
	{
		this->value_ = allocateStorage( type_, size_ );
		memcpy( value_, other.value_, sizeTable_[type_] * size_ );
	}
}

void Parameter::operator=( const Parameter& other )
{
	this->type_ = other.type_;
	this->value_ = other.value_;
	this->size_ = other.size_;
	this->name_ = other.name_;
	this->isArrayChild_ = other.isArrayChild_;
	if( isArrayChild_ )
		this->value_ = other.value_;
	else
	{
		this->value_ = allocateStorage( type_, size_ );
		memcpy( value_, other.value_, sizeTable_[type_] * size_ );
	}
}

Parameter Parameter::operator[]( unsigned int index ) throw( IndexOutOfRangeException )
{
	if( index >= size_ ) 
		BOOST_THROW_EXCEPTION(IndexOutOfRangeException() << 
				IndexOutOfRangeException::RequestedIndex(index) <<
				IndexOutOfRangeException::ArraySize(size_) << 
				ParameterException::ParameterName(name()));
	Parameter childParameter;
	childParameter.size_ = 1;
	childParameter.type_ = type_;
	childParameter.value_ = (void*)((char*)value_ + index * sizeTable_[type_]);
	childParameter.isArrayChild_ = true;
	childParameter.name_ = name_ + "[" + boost::lexical_cast<std::string>( index ) + "]";
	return childParameter;
}

Parameter::operator int() throw(InvalidParameterCastException)
{
	if( type_ == Int )
		return ((int*) value_)[0];
	else if( type_ == Float )
		return (int)((float*) value_)[0];
	else BOOST_THROW_EXCEPTION(InvalidParameterCastException() << 
			InvalidParameterCastException::FromType("int") << 
			InvalidParameterCastException::ToType(nameFromType(type_)) <<
			ParameterException::ParameterName(name_));
}

Parameter::operator float() throw(InvalidParameterCastException)
{
	if( type_ == Float )
		return ((float*) value_)[0];
	else if( type_ == Int )
		return (float)((int*) value_)[0];
	else BOOST_THROW_EXCEPTION(InvalidParameterCastException() << 
			InvalidParameterCastException::FromType("float") << 
			InvalidParameterCastException::ToType(nameFromType(type_)) <<
			ParameterException::ParameterName(name_));
}

#define PARAMETER_VALUE_CAST( T, N ) \
	Parameter::operator T&() throw(InvalidParameterCastException)\
	{\
		if( type_ == N ) return ((T*) value_)[0];\
		else BOOST_THROW_EXCEPTION(InvalidParameterCastException() << \
			InvalidParameterCastException::FromType(#T) << \
			InvalidParameterCastException::ToType(nameFromType(type_)) << \
			ParameterException::ParameterName(name_));\
	}

PARAMETER_VALUE_CAST( vec2, Vec2 )
PARAMETER_VALUE_CAST( vec3, Vec3 )
PARAMETER_VALUE_CAST( vec4, Vec4 )
PARAMETER_VALUE_CAST( mat3, Mat3 )
PARAMETER_VALUE_CAST( mat4, Mat4 )

#define PARAMETER_POINTER_CAST( T, N ) \
	Parameter::operator T*() throw(InvalidParameterCastException)\
	{\
		if( type_ == N ) return (T*) value_;\
		else BOOST_THROW_EXCEPTION(InvalidParameterCastException() << \
			InvalidParameterCastException::FromType(#T) << \
			InvalidParameterCastException::ToType(nameFromType(type_)) << \
			ParameterException::ParameterName(name_));\
	}

PARAMETER_POINTER_CAST( int, Int )
PARAMETER_POINTER_CAST( float, Float )
PARAMETER_POINTER_CAST( vec2, Vec2 )
PARAMETER_POINTER_CAST( vec3, Vec3 )
PARAMETER_POINTER_CAST( vec4, Vec4 )
PARAMETER_POINTER_CAST( mat3, Mat3 )
PARAMETER_POINTER_CAST( mat4, Mat4 )

#define PARAMETER_VALUE_SET( T, N ) \
	void Parameter::operator=( const T& value ) throw(InvalidParameterCastException)\
	{\
		if( type_ == N ) ((T*) value_)[0] = value;\
		else BOOST_THROW_EXCEPTION(InvalidParameterCastException() << \
			InvalidParameterCastException::FromType(#T) << \
			InvalidParameterCastException::ToType(nameFromType(type_)) << \
			ParameterException::ParameterName(name_));\
	}

PARAMETER_VALUE_SET( int, Int )
PARAMETER_VALUE_SET( float, Float )
PARAMETER_VALUE_SET( vec2, Vec2 )
PARAMETER_VALUE_SET( vec3, Vec3 )
PARAMETER_VALUE_SET( vec4, Vec4 )
PARAMETER_VALUE_SET( mat3, Mat3 )
PARAMETER_VALUE_SET( mat4, Mat4 )
