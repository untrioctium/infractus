void main()
{
	vec2 startValue = texture( intex, gl_TexCoord[0].st ).xy;
	
	vec2 value;
	vec2 lastValue = startValue;
	float valueLength;
	
	for( int i = 1; i < maxIterations; i++ )
	{
		value.x = $x$;
		value.y = $y$;
		
		valueLength = length(value);
		if( valueLength > escapeRadius )
		{
			gl_FragColor = vec4( i, 0.0, 0.0, 0.0 );
			return;
		}
		lastValue = value;
	}
	
	gl_FragColor = vec4( -1, 0.0, 0.0, 0.0 );
}
