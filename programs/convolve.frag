vec2 offset[9] =vec2[]( 
			vec2(-DX, -DY), vec2(0.0, -DY), vec2(DX, -DY), 
			vec2(-DX, 0.0), vec2(0.0, 0.0), vec2(DX, 0.0), 
			vec2(-DX, DY), vec2(0.0, DY), vec2(DX, DY) 
			);

void main()
{
	vec4 sum = vec4(0.0);
	vec4 src;
	float kernelSum = convolveKernel[0] + convolveKernel[1] + convolveKernel[2] + 
						convolveKernel[3] + convolveKernel[4] + convolveKernel[5] + 
						convolveKernel[6] + convolveKernel[7] + convolveKernel[8];
	
	vec4 tmp = texture2D(intex, gl_TexCoord[0].st + offset[0]);
	sum += tmp * convolveKernel[0];
	tmp = texture2D(intex, gl_TexCoord[0].st + offset[1]);
	sum += tmp * convolveKernel[1];
	tmp = texture2D(intex, gl_TexCoord[0].st + offset[2]);
	sum += tmp * convolveKernel[2];
	tmp = texture2D(intex, gl_TexCoord[0].st + offset[3]);
	sum += tmp * convolveKernel[3];
	src = texture2D(intex, gl_TexCoord[0].st + offset[4]);
	sum += src * convolveKernel[4];
	tmp = texture2D(intex, gl_TexCoord[0].st + offset[5]);
	sum += tmp * convolveKernel[5];
	tmp = texture2D(intex, gl_TexCoord[0].st + offset[6]);
	sum += tmp * convolveKernel[6];
	tmp = texture2D(intex, gl_TexCoord[0].st + offset[7]);
	sum += tmp * convolveKernel[7];
	tmp = texture2D(intex, gl_TexCoord[0].st + offset[8]);
	sum += tmp * convolveKernel[8];
	
	if( kernelSum != 0 )
		sum /= kernelSum;
		
	gl_FragColor = sum;//mix(sum, src, 1 - sum.a/pow(sum.a, mixPow));//vec4(sum.xyz, srcAlpha);
}
