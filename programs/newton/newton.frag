#pragma include <complex>
#line 2

void main()
{
	vec2 startValue = texture2D( intex, gl_TexCoord[0].xy ).xy;
	vec2 lastValue = startValue;
	vec2 currentValue;
	float curveLength = 0;

	int iterationCount;
	float delta = 1;
	float lastDelta;
	float convergeAngle;
	vec2 value;
	float angAvg = 0;

	for( iterationCount = 1; iterationCount < maxIterations; iterationCount++ )
	{
		currentValue = vec2(0.0);
		for( int i = 0; i < functionSize; i++ )
		{
			currentValue += cDiv( function[i].zw, lastValue - function[i].xy );
		}
		currentValue = lastValue - cDiv( coeff, currentValue );
		lastDelta = delta;
		delta = cAbs( currentValue - lastValue );
		curveLength += delta;
		if( delta <= requiredPrecision )
		{
			convergeAngle = cArg( lastValue - currentValue );
			break;
		}
		lastValue = currentValue;
	}

	float fractionalIter = iterationCount;
	if( fractionalIter > maxIterations ) fractionalIter = maxIterations;

	gl_FragColor = vec4( atan(currentValue.y, currentValue.x), curveLength, fractionalIter, (convergeAngle + PI)/(2 * PI) );
}
