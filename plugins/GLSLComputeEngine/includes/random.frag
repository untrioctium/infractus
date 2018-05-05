#extension GL_EXT_gpu_shader4 : enable

uvec4 randState;
uint rand()
{
	uint t = randState.x ^ (randState.x << 11);
	randState.x = randState.y; 
	randState.y = randState.z; 
	randState.z = randState.w;
	randState.w = randState.w ^ (randState.w >> 19) ^ t ^ (t >> 8);
	return randState.w;
}

void srand(uint seed)
{
	randState.x = uint(123456789);
	randState.y = uint(362436069);
	randState.z = uint(521288629);
	randState.w = seed;
	randState = uvec4( rand(), rand(), rand(), rand() );
}

uint threadSeed()
{	
	uint seed = uint(floor(gl_TexCoord[0].y/( DY * DX ) * gl_TexCoord[0].x/DX));
	return seed * seed + seed;
}

float randFloat(float div)
{
	return fract(float(rand())/div);
}

float randFloat()
{
	return randFloat(6596554.51657487);
}
