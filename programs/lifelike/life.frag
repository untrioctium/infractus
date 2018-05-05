void main()
{
	int age = int(texture2D( intex, gl_TexCoord[0].xy ).x);
	float death = texture2D( intex, gl_TexCoord[0].xy ).y;
	int neighborCount = 0;
	vec2 coord;
	
	for( int y = -1; y != 2; y++ )
		for( int x = -1; x != 2; x++ )
		{
			coord = fract(gl_TexCoord[0].xy + vec2( DX * x, DY * y ));
			if( coord.x < 0 ) coord.x = 1 - coord.x;
			if( coord.x > 1 ) coord.x = fract(coord.x);
			if( coord.y > 1 ) coord.y = fract(coord.y);
			if( coord.y < 0 ) coord.y = 1 - coord.y;
			else neighborCount += (texture(intex, coord).x > 0.0)? 1: 0;
		}
	
	if( age > 0 ) neighborCount--;
	
	if( death > 0 )
		death--;
	else
	{
		if( age > 0.0 )
			if( ((1 << neighborCount) & liveRules) > 0 )
				age++;
			else
			{
				age = -age;
				death = deathStates;
			}
		else
			if( ((1 << neighborCount) & birthRules) > 0)
				age = 1;
			else
				age = 0;
	}

	gl_FragColor = vec4( age, death, 0, 0 );
}
