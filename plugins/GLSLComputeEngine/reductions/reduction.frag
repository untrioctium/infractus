uniform float xOff;
uniform float yOff;
uniform float width;
uniform float height;
uniform sampler2D intex;

void reduce( inout vec4 a, in vec4 b );

void main()
{
	vec2 coord = gl_TexCoord[0].xy;
	vec4 value = texture( intex, coord );
	
	vec2 offCoord = vec2(coord.x + xOff, coord.y + yOff);// + vec2( xOff, yOff );
	
	bool checkCorner = true;
	if( offCoord.x < 1 && xOff != 0.0 )
		reduce( value, texture( intex, vec2( offCoord.x, coord.y ) ));
	else checkCorner = false;
		
	if( offCoord.y < 1 && yOff != 0.0  )
		reduce( value, texture( intex, vec2( coord.x, offCoord.y ) ));
	else checkCorner = false;
	
	if( checkCorner )
		reduce( value, texture( intex, offCoord ));
		
	gl_FragColor = value;
}
