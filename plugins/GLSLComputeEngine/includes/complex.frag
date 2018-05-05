/**
 * @file complex.frag
 * @brief A set of functions for dealing with complex numbers in GLSL.
 */
#define PI 3.14159265358979323846264

/**
 * @brief Adds two complex numbers.
 * @param a The first complex number.
 * @param b The second complex number.
 * @return \f$a + b\f$
 *
 * This function is technically not needed, as the addition of vectors
 * is equivilent to the addition of complex numbers.
 */
vec2 cAdd( vec2 a, vec2 b )
{
	return a + b;
}

/**
 * @brief Subtracts two complex numbers.
 * @param a The minuend.
 * @param b The subtrahend.
 * @return \f$a - b\f$
 *
 * This function is technically not needed, as the subtraction of vectors
 * is equivilent to the addition of complex numbers.
 */
vec2 cSub( vec2 a, vec2 b )
{
	return a - b;
}

/**
 * @brief Multiplies two complex numbers.
 * @param a The first complex number.
 * @param b The second complex number.
 * @return \f$a * b\f$
 */
vec2 cMul( vec2 a, vec2 b )
{
	return vec2( a.x * b.x - a.y * b.y, a.y * b.x + a.x * b.y );
}

/**
 * @brief Calculates the inverse of a complex number.
 * @param z The complex number.
 * @return \f$\frac{1}{z}\f$
 *
 * Use this instead of cDiv if you only wish to find the inverse,
 * as this will be faster in the case where the numerator is 1.
 */
vec2 cInv( vec2 z )
{
	return vec2( z.x, -z.y )/(z.x*z.x + z.y * z.y);
}

/**
 * @brief Divides two complex numbers.
 * @param a The numerator.
 * @param b The denominator.
 * @return \f$\frac{a}{b}\f$
 */
vec2 cDiv( vec2 a, vec2 b )
{
	return vec2( a.x * b.x + a.y * b.y, a.y * b.x - a.x * b.y )/(b.x*b.x + b.y * b.y);
}

/**
 * @brief Calculates complex exponentation.
 * @param a The base.
 * @param b The exponent.
 * @return \f$a^b\f$
 */
vec2 cPow( vec2 a, vec2 b )
{
	float r = sqrt(a.x * a.x + a.y * a.y);
	float theta = atan(a.y, a.x);

	float arg = b.y * log(r) + b.x * theta;
	return pow( r, b.x ) * exp( -b.y * theta ) * vec2( cos(arg), sin(arg) ); 
}

/**
 * @brief Calculates complex exponentation with a real exponent.
 * @param a The base.
 * @param b The exponent.
 * @return \f$a^b\f$
 *
 * Use this overload if you know the exponent will be real, as it shaves off
 * a few calculations versus raising to a complex power.
 */
vec2 cPow( vec2 a, float b )
{
	float arg = b * atan(a.y, a.x);
	return pow( sqrt(a.x * a.x + a.y * a.y), b ) * vec2( cos(arg), sin(arg) );
}

/**
 * @brief Calculates complex exponentation with a real base.
 * @param a The base.
 * @param b The exponent.
 * @return \f$a^b\f$
 *
 * Use this overload if you know the base will be real, as it shaves off
 * a few calculations versus using a complex base.
 */
vec2 cPow( float a, vec2 b )
{
	float r = abs(a);

	float arg = b.y * log(r);
	return pow( r, b.x ) * vec2( cos(arg), sin(arg) );
}

/**
 * @brief Calculates complex exponentation in base \f$e\f$.
 * @param z The exponent.
 * @return \f$e^z\f$
 */
vec2 cExp( vec2 z )
{
	return exp(z.x) * vec2( cos(z.y), sin(z.y) ); 	
}

/**
 * @brief Calculates the complex logarithm base \f$e\f$.
 * @param z The complex number.
 * @return \f$log_e(z)\f$
 */
vec2 cLog( vec2 z )
{
	return vec2( log( z.x * z.x + z.y * z.y ), atan( z.y, z.x ) );
}

/**
 * @brief Calculates the complex sine.
 * @param z The complex number.
 * @return \f$sin(z)\f$
 */
vec2 cSin( vec2 z )
{
	return vec2( sin(z.x) * cosh(z.y), cos(z.x) * sinh(z.y));
}

/**
 * @brief Calculates the complex cosine.
 * @param z The complex number.
 * @return \f$cos(z)\f$
 */
vec2 cCos( vec2 z )
{
	return vec2( cos(z.x) * cosh(z.y), -sin(z.x) * sinh(z.y));
}

/**
 * @brief Calculates the commplex tangent.
 * @param z The complex number.
 * @return \f$tan(z)\f$
 */
vec2 cTan( vec2 z )
{
	return cDiv( cSin(z), cCos(z) );
}

/**
 * @brief Calculates the magnitude of a complex number.
 * @param z The complex number.
 * @return The magnitude of \a z.
 */
float cAbs( vec2 z )
{
	return sqrt( z.x * z.x + z.y * z.y );
}

/**
 * @brief Calculates the argument of a complex number.
 * @param z The complex number.
 * @return The angle made by \a z and the origin, in the range \f$[-\pi,\pi]\f$.
 */
float cArg( vec2 z )
{
	return atan( z.y, z.x );
}

vec2 cToPolar( vec2 z )
{
	return vec2( length(z), cArg(z) );
}
