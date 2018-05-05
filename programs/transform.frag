#pragma include <complex>

void main()
{
	float yRange = Y_MAX - Y_MIN;
	float xRange = X_MAX - X_MIN;

	vec2 z = vec2( gl_TexCoord[0].x * xRange + X_MIN, gl_TexCoord[0].y * yRange + Y_MIN );
	z = z * blend + (1 - blend) * $TRANSFORM$;
	
	gl_FragColor = vec4( z, 0, 0 );
}
