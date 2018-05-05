float PI = 3.14159265358979323846264;

vec3 hsvToRGB( vec3 hsv )
{
	if(hsv.r == 1) hsv.r = 0;
	
	int i = int(floor(hsv.r*6));
	float f = hsv.r*6 - floor(hsv.r*6);
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

void main()
{
	vec4 pixelData = texture2D( intex, gl_TexCoord[0].xy );

	float age = abs(pixelData.x);
	float death = pixelData.y;

	vec3 hsv;
	hsv.x = sin(pixelData.x / hueSpacing * PI) * .5 + .5;
	hsv.y = (pixelData.x > 0.0)?1 - 1/(1 + .025*pixelData.x): 0.0;
	hsv.z = (pixelData.x > 0.0)? hsv.y: 0.0;
	
	if( death > 0 )
	{
		hsv.y *= death/deathStates;
		hsv.z *= death/deathStates;
	}
	gl_FragColor = vec4( hsvToRGB(hsv), 1.0 );
}
