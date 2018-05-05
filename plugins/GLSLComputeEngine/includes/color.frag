/**
 * @file color.frag
 * @brief A set of functions for dealing with various color spaces in GLSL.
 */

/**
 * @brief Converts a HSV triplet to a RGB triplet.
 * @param hsv A vec3 with the hue in the first element, the saturation 
 *	      in the second, and the value in the third.
 * @return The converted RGB value.
 *
 * Hue, saturation, and value must be in the range [0,1] for the function
 * to work properly.
 */
vec3 hsvToRGB( vec3 hsv )
{
	if(hsv.r == 1) hsv.r = 0;
	
	int i = int(floor(hsv.r*6));
	float f = hsv.r*6 - i;
	float p = (hsv.b * (1 - hsv.g));
	float q = hsv.b * (1 - f * hsv.g);
	float t = hsv.b * (1 - (1-f) * hsv.g);
	float v = hsv.b;

	switch(i)
	{
		case 0:
			return vec3(v,t,p);
		case 1:
			return vec3(q,v,p);
		case 2:
			return vec3(p,v,t);
		case 3:
			return vec3(p,q,v);
		case 4:
			return vec3(t,p,v);
		case 5:
			return vec3(v,p,q);
	}
	return vec3(0,0,0);
}
