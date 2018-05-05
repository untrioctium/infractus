#pragma include <random>
#line 2

#define PI 3.14159265358979323846264
#define SQRT_2 1.4142135623730950488
#define NUDGE 1e-6
#define EPS 2e-24
#define LOG_10 2.302585092994045684017991454684360

#define DO_VARIATIONS 1

#define BADVALUE(x) (((x)!=(x))||((x)>1e10)||((x)<-1e10))

vec2 symPass( vec2 vec, int funcIdx )
{
	vec2 p;
	mat4 mat = mats[funcIdx];
	p.x = mat[0].x * vec.x + mat[0].y * vec.y;
	p.y = mat[1].x * vec.x + mat[1].y * vec.y;
	return p;
}

vec2 flame_pass( vec2 vec, int funcIdx )
{
	mat4 mat = mats[funcIdx];
	
	vec2 p;
	p.x = mat[0].x * vec.x + mat[0].y * vec.y + mat[0].z;
	p.y = mat[1].x * vec.x + mat[1].y * vec.y + mat[1].z;
	
	vec2 result = vec2(0);
	
	#if DO_VARIATIONS == 1 
	
		result = vec2(0.0, 0.0);
			
		float r = length(p);
		float rSq = r * r;
		
		float a = atan(p.x/p.y);
		float t = atan(p.y/p.x);
		
		if( varOffsets[funcIdx] < 0 ) result = p;
		else
		{
			for( int i = varOffsets[funcIdx]; vars[i].x >= 0; i++ )
			{
				float weight = vars[i].y;
				int idx = floor(vars[i].x);
				$VARSOURCE$
			} // end loop
		} // end if
	#else
		result = p;
	#endif
	
	p.x = mat[2].x * result.x + mat[2].y * result.y + mat[2].z;
	p.y = mat[3].x * result.x + mat[3].y * result.y + mat[3].z;
	return p;
	
}

void main()
{
	srand(threadSeed() + time);
	
	vec2 pos;
	float col;
	
	if( seed == 1 )
	{
		pos = vec2(randFloat(), randFloat()) * 2 - 1;
		col = .5;
	}
	else
	{
		vec4 inputVec = texture( input[0], gl_TexCoord[0].xy );
		pos = inputVec.xy;
		col = inputVec.w;
	}
	
	float func, sum;
	int invalidIterations = 0;
	uint funcIdx;
	vec2 newPos;
	vec3 funcColor;
	
	// number of symmetry planes: symmetry
	// rotation per plane: 360.0/(symmetry + 1)
	
	for( int i = 0; i < iterCount; i++ )
	{
		func = randFloat() * densitySum;
		sum = 0;

		for( int i = 0; i < matCount; i++ )
		{
			sum += mats[i][3].w;
			if( func < sum || i + 1 == matCount )
			{
				funcIdx = i;
				break;
			}
		}
	
		funcColor = mats[funcIdx][0].w;
		pos = flame_pass( pos, funcIdx );
		col = mix(col, funcColor, colSpeeds[funcIdx]);	

	}
	gl_FragData[0] = vec4( pos, 0.0 , col );
	gl_FragData[1] = vec4( texture(input[1], vec2(.5, col)));
}
