namespace glUniform
{
	typedef void (*TypeFunction)( GLuint, Parameter& );

	void Float( GLuint location, Parameter& parameter )
	{
		glUniform1fvARB( location, parameter.size(), (float*) parameter );
	}

	void Int( GLuint location, Parameter& parameter )
	{
		glUniform1ivARB( location, parameter.size(), (int*) parameter );
	}

	void Vec2( GLuint location, Parameter& parameter )
	{
		glUniform2fvARB( location, parameter.size(), (GLfloat*)((vec2*) parameter) );
	}

	void Vec3( GLuint location, Parameter& parameter )
	{
		glUniform3fvARB( location, parameter.size(), (GLfloat*)((vec3*) parameter) );
	}

	void Vec4( GLuint location, Parameter& parameter )
	{
		// glUniform4fv expects the third parameter to be a float array
		// with the second parameter being the size of the array divided by 4.
		// since the vec4 array is essentially an array of floats when laid out 
		// in memory we can just pass the vec4 array pointer in.
		//
		// this is somewhat of an ugly hack, but I like it. :)
		glUniform4fvARB( location, parameter.size(), (GLfloat*)((vec4*) parameter) );
	}
	
	void Mat3( GLuint location, Parameter& parameter )
	{
		glUniformMatrix3fvARB( location, parameter.size(), 0, (GLfloat*)((mat3*) parameter) );
	}

	void Mat4( GLuint location, Parameter& parameter )
	{
		glUniformMatrix4fvARB( location, parameter.size(), 0, (GLfloat*)((mat4*) parameter) );
	}

	std::map<Parameter::ParameterType, TypeFunction> push;

	void Initalize()
	{
		push[Parameter::Float] = &Float;
		push[Parameter::Int] = &Int;
		push[Parameter::Vec2] = &Vec2;
		push[Parameter::Vec3] = &Vec3;
		push[Parameter::Vec4] = &Vec4;
		push[Parameter::Mat3] = &Mat3;
		push[Parameter::Mat4] = &Mat4;
	}
};
