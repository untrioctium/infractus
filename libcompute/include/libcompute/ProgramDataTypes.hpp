/**
 * @file ProgramDataTypes.hpp
 * @brief Header file that defines the types that can be used by programs.
 */

#include <cmath>

namespace libcompute
{

/**
 * @brief The base vector class that defines operations that all derived vectors should have.
 * @tparam vecN The type of the derived vector.
 */
template<class vecN> class vecBase
{
public:
	/** Gets the magnitude (ie length) of this vector. */
	float mag() const;
	
	/** Returns a vector with length one that is in the same direction as this vector. */
	vecN normalize() const;
	
	/** Returns this vector divided by a scalar. */
	vecN operator/(float scalar) const;
	
	/** Returns this vector multiplied by a scalar. */
	vecN operator*(float scalar) const;
	
	/** Performs in-place division by a scalar. */
	void operator/=( float scalar );
	
	/** Performs in-place multiplication by a scalar. */
	void operator*=( float scalar );
	
	/** Returns this vector added to another vector. */
	vecN operator+( const vecN& other ) const;
	
	/** Returns the subtraction of another vector from this vector. */
	vecN operator-( const vecN& other ) const;
	
	/** Performs in-place addition with another vector. */	
	void operator+=( const vecN& other );
	
	/** Performs in-place subtraction with another vector. */
	void operator-=( const vecN& other );
	
	/** Returns the distance between this vector and another. */
	float distance( const vecN& other ) const;
	
	/** Calculates the dot product of this vector and another. */
	float dot( const vecN& other ) const;
	
	/** Returns the scalar composition of this vector onto \a other. */
	float comp( const vecN& other ) const;
	
	/** Returns the vector projection of this vector onto \a other. */
	vecN proj( const vecN& other ) const;
};

/** A two dimensional floating point vector */
class vec2: public vecBase<vec2>
{
public:
	
	float x; ///< The x component of this vector
	float y; ///< The y component of this vector

	/**
	 * @brief Create a vector with the given component values.
	 * @param X The x component.
	 * @param Y The y component.
	 */ 
	vec2( float X, float Y )
	: x(X)
	, y(Y) {}

	/** Default constructor that sets the vector to \f$<0,0>\f$ */
	vec2()
	: x(0)
	, y(0) {}

	/** 
	 * @brief Returns the argument (ie angle) of this vector about the origin.
	 * @return The argument in the range \f$[-\pi,\pi]\f$.
	 */
	float arg() const { return atan2(y,x); }
	
	float mag() const { return sqrt(x * x + y * y); }
	vec2 normalize() const { return (*this)/mag(); }
	vec2 operator/(float scalar) const { return vec2(x/scalar, y/scalar); }
	vec2 operator*(float scalar) const { return vec2(x*scalar, y*scalar); }
	void operator/=( float scalar ) { x/=scalar; y/=scalar; }
	void operator*=( float scalar ) { x*=scalar; y*=scalar; }
	vec2 operator+( const vec2& o ) const { return vec2(x+o.x, y+o.y); }
	vec2 operator-( const vec2& o ) const { return vec2(x-o.x, y-o.y); }
	void operator+=( const vec2& o ) { x+=o.x; y+=o.y; }
	void operator-=( const vec2& o ) { x-=o.x; y-=o.y; }
	float distance( const vec2& o ) const { return (*this - o).mag(); }
	float dot( const vec2& o ) const { return x*o.x + y*o.y; }
	float comp( const vec2& o ) const { return dot(o)/mag(); }
	vec2 proj( const vec2& o ) const { return normalize() * comp(o); }
};

/** A three dimensional floating point vector */
class vec3: public vecBase<vec3>
{
public:
	float x; ///< The x component of this vector
	float y; ///< The y component of this vector
	float z; ///< The z component of this vector

	/**
	 * @brief Create a vector with the given component values.
	 * @param X The x component.
	 * @param Y The y component.
	 * @param Z The z component.
	 */ 
	vec3( float X, float Y, float Z )
	: x(X)
	, y(Y)
	, z(Z) {}

	/** Default constructor that sets the vector to \f$<0,0,0>\f$ */
	vec3()
	: x(0)
	, y(0)
	, z(0) {}
	
	/** Returns the cross product of this and \a other. */
	vec3 crossProduct( const vec3& other ) const;
	
	float mag() const { return sqrt(x * x + y * y + z*z); }
	vec3 normalize() const { return (*this)/mag(); }
	vec3 operator/(float scalar) const { return vec3(x/scalar, y/scalar,z/scalar); }
	vec3 operator*(float scalar) const { return vec3(x*scalar,y*scalar,z*scalar); }
	void operator/=( float scalar ) { x/=scalar; y/=scalar; z/=scalar; }
	void operator*=( float scalar ) { x*=scalar; y*=scalar; z*=scalar; }
	vec3 operator+( const vec3& o ) const { return vec3(x+o.x, y+o.y, z+o.z); }
	vec3 operator-( const vec3& o ) const { return vec3(x-o.x, y-o.y, z-o.z); }
	void operator+=( const vec3& o ) { x+=o.x; y+=o.y; z+=o.z; }
	void operator-=( const vec3& o ) { x-=o.x; y-=o.y; z-=o.z; }
	float distance( const vec3& o ) const { return (*this - o).mag(); }
	float dot( const vec3& o ) const { return x*o.x + y*o.y + z*o.z; }
	float comp( const vec3& o ) const { return dot(o)/mag(); }
	vec3 proj( const vec3& o ) const { return normalize() * comp(o); }
};

/** A four dimensional floating point vector */
class vec4: public vecBase<vec4>
{
public:
	float x; ///< The x component of this vector
	float y; ///< The y component of this vector
	float z; ///< The z component of this vector
	float w; ///< The w component of this vector

	/**
	 * @brief Create a vector with the given component values.
	 * @param X The x component.
	 * @param Y The y component.
	 * @param Z The z component.
	 * @param W The w component.
	 */ 
	vec4( float X, float Y, float Z, float W )
	: x(X)
	, y(Y)
	, z(Z)
	, w(W) {}

	/** Default constructor that sets the vector to \f$<0,0,0,0>\f$ */
	vec4()
	: x(0)
	, y(0)
	, z(0)
	, w(0) {}

	float mag() const { return sqrt(x * x + y * y + z*z + w*w); }
	vec4 normalize() const { return (*this)/mag(); }
	vec4 operator/(float scalar) const { return vec4(x/scalar, y/scalar,z/scalar,w/scalar); }
	vec4 operator*(float scalar) const { return vec4(x*scalar,y*scalar,z*scalar,w*scalar); }
	void operator/=( float scalar ) { x/=scalar; y/=scalar; z/=scalar;w/=scalar; }
	void operator*=( float scalar ) { x*=scalar; y*=scalar; z*=scalar;z*=scalar; }
	vec4 operator+( const vec4& o ) const { return vec4(x+o.x, y+o.y, z+o.z, w+o.w); }
	vec4 operator-( const vec4& o ) const { return vec4(x-o.x, y-o.y, z-o.z, w-o.w); }
	void operator+=( const vec4& o ) { x+=o.x; y+=o.y; z+=o.z; w+=o.w; }
	void operator-=( const vec4& o ) { x-=o.x; y-=o.y; z-=o.z; w+=o.w; }
	float distance( const vec4& o ) const { return (*this - o).mag(); }
	float dot( const vec4& o ) const { return x*o.x + y*o.y + z*o.z + w*o.w; }
	float comp( const vec4& o ) const { return dot(o)/mag(); }
	vec4 proj( const vec4& o ) const { return normalize() * comp(o); }
};

template<int SIZE> class matBase
{
public:
	matBase()
	{
		for( unsigned int i = 0; i < SIZE * SIZE; i++ )
			data_[i] = 0;
		
		for( unsigned int i = 0; i < SIZE * SIZE; i += SIZE + 1 )
			data_[i] = 1;
	}
	
	float& operator()( int r, int c )
	{
		return data_[r * SIZE + c];
	}
	
	void operator=(const matBase& other)
	{
		for( unsigned int i = 0; i < SIZE * SIZE; i++ )
			data_[i] = other.data_[i];
	}
	
	matBase operator*(float factor)
	{
		matBase ret;
		for( unsigned int i = 0; i < SIZE * SIZE; i++ )
			ret.data_[i] = data_[i] * factor;
		
		return ret;
	}
	
	matBase operator+(const matBase& other)
	{
		matBase ret;
		for( unsigned int i = 0; i < SIZE * SIZE; i++ )
			ret.data_[i] = other.data_[i] + data_[i];
		
		return ret;
	}
	
	matBase operator-(const matBase& other)
	{
		matBase ret;
		for( unsigned int i = 0; i < SIZE * SIZE; i++ )
			ret.data_[i] = data_[i] - other.data_[i];
		
		return ret;
	}
	
private:

	float data_[SIZE * SIZE];
};

typedef matBase<3> mat3;
typedef matBase<4> mat4;
};
