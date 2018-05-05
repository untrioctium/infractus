float PI = 3.14159265358979323846264;

// converts a hue, saturation, value triplet to a RGB triplet
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

	float rootArg = pixelData.x;
	float curveLength = pixelData.y;
	float iterCount = pixelData.z;
	float convergeAngle = pixelData.w;

	float rat = iterCount/maxIterations;
	float logRat = 1 - 1/ (.2 * curveLength + 1.0) + iterCount;//log(iterCount)/log(maxIterations);
	logRat = log(logRat) / log(float(maxIterations));

	vec3 hsv;

	hsv.r = .1 * curveLength/ (.1 * curveLength + 1.0);//(rootArg + PI)/(2 * PI) + hueOffset;
	hsv.r = hsv.r - floor(hsv.r);
	hsv.g = logRat;
	hsv.b = 1 - logRat;

	vec3 rgb = hsvToRGB(hsv);
		
	hsv.r = (rootArg + PI)/(2*PI) + hueOffset;
	hsv.r = hsv.r - floor(hsv.r);
	rgb = rgb * blend +  hsvToRGB(hsv) * (1 - blend);
	
	rgb.r = pow( rgb.r, 1/gamma.r );
	rgb.g = pow( rgb.g, 1/gamma.g );	
	rgb.b = pow( rgb.b, 1/gamma.b );
	
	gl_FragColor = vec4( rgb, 1.0 );

}
