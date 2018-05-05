void main()
{
	vec2 coords = gl_TexCoord[0].st;
	
	vec4 avg = vec4(0.0);
	float div = samples * samples;
	
	for( int y = 0; y < samples; y++ )
		for( int x = 0; x < samples; x++ )
			avg += texture(input, coords + vec2( inOff.x * x, inOff.y * y ) )/div;
	
	gl_FragColor = avg;	
}
