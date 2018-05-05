void main()
{
	vec3 color = texture2D(intex, gl_TexCoord[0].st).rgb;
	float scale;

	if( useAlpha == 1 ) 
		scale = texture2D(intex, gl_TexCoord[0].st).a;
	else scale = denom;
	
	vec3 powpregamma = vec3(preGamma, preGamma, preGamma);
	vec3 powpostgamma = vec3(postGamma, postGamma, postGamma);

	// get rid of nans or infinities, else there will be lovely
	// glaring discolored pixels in the final image
	color = clamp( color, 0, scale );
	//color = mix( color, vec3(0.0), isinf(color) || isnan(color) );
	color = pow(color, powpregamma);
	color *= exposure;
	color *= log(scale)/(scale);
	color = clamp( color, 0, 1);
	color = pow(color, powpostgamma);
	
	gl_FragColor = vec4( color, alpha );
}
