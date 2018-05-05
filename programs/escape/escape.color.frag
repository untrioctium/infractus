#pragma include <color>

void main()
{
	vec4 results = texture( intex, gl_TexCoord[0].st );
	float iterCount = results.x;
	
	if( iterCount == -1 )
	{
		if( highlightNonConverge == 1 )
		{
			gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			return;
		}
		iterCount = 0;
	}
	
	vec3 hsv;
	
	float logRat = log(iterCount)/log(float(maxIterations));
	hsv.r = fract(logRat + hueOffset);
	hsv.g = logRat;
	hsv.b = logRat;
	
	gl_FragColor = vec4( hsvToRGB(hsv), 1.0 );
}
