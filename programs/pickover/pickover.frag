#pragma include <color>
#pragma include <complex>
#pragma include <random>
#line 5

void main()
{
	uint seed = threadSeed();
	srand(seed * seed + uint(time));
	
	vec3 cur;
	vec3 prev;
	vec3 next = vec3( gl_TexCoord[0].xy * 2 - 1, randFloat() * 2 - 1);

	float color = 0.5;
	float escapeCount = 0;

	for( int i = 0; i < iterCount; i++ )
	{
		prev = cur;
		cur = next;
		$SOURCE$
		
		float lnext = length(next);
		if( lnext > escapeRadius ) 
		{
			float p = 1 - escapeRadius * (lnext/escapeRadius + 1)/lnext;
			next *= p;
			lnext *= -p;
			escapeCount+=1.0;
		}
		color = (color + lnext)/2.0;
	}
	
	vec3 curnext = next - cur;
	vec3 prevcur = prev - cur;
	float lcurnext = length(curnext);
	float lprevcur = length(prevcur);
	float len = lcurnext + lprevcur;
	float angle = dot( curnext, prevcur )/(lcurnext * lprevcur);
	angle = clamp( angle, 0.0, 1.0 );
	angle = pow(acos(angle)/PI, anglePow);
		
	len = pow( len/(1 + len), lenPow );
	
	vec3 hsv;
	hsv.r = (angle + len)/2;
	hsv.g = 1 - exp( - hsv.r * hsv.r * 2.5 );
	hsv.r = fract(pow(color, hsv.r) + hueOffset);
	hsv.b = 1;
	
	gl_FragData[0] = vec4(next, 1.0);
	gl_FragData[1] = vec4(hsvToRGB(hsv), alpha );
}
