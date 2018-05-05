uniform sampler2D vertexColors;
varying vec2 texCoord;

void main()
{
	gl_FragColor = texture2D(vertexColors, texCoord);
}
