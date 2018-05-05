void reduce( inout vec4 a, in vec4 b )
{
	a.x = max( a.x, b.x );
	a.y = max( a.y, b.y );
	a.z = max( a.z, b.z );
	a.w = max( a.w, b.w );
}
