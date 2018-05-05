void main()
{
	gl_FragColor = 1.0 - texture( input, gl_TexCoord[0].xy);
}
