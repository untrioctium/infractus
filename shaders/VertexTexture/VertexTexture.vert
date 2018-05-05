uniform sampler2D vertexPositions;

varying vec2 texCoord;

void main()
{
	//srand(threadSeed());
	texCoord = gl_Vertex.xy;
	vec4 pos = texture2D( vertexPositions, texCoord ).xyzz;
	pos.w = 1.0;
	//float r = length(pos.xy);
	//pos.xy = 2.0/( r + 1.0 ) * pos.yx;
	gl_Position = gl_ModelViewProjectionMatrix * pos;
}
