#include <any>
#include <libcompute.hpp>
using namespace libcompute;

#include <stdlib.h>
#include <GL/glew.h>
#include <stdlib.h>
#include <fstream>
#include <cstring>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "glUniform.hpp"

void checkGLErrors(const char *label) {
    GLenum errCode;
    const GLubyte *errStr;
    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errStr = gluErrorString(errCode);
        printf("OpenGL ERROR: %s (Label: %s)\n", (char*)errStr, label);
    }
}

class GLSLComputeEngine: public Engine
{
public:

	GLSLComputeEngine() : Engine( "GLSLComputeEngine") { init(); }
	~GLSLComputeEngine() {};

	class DataStorage: public Engine::DataStorage
	{
	public:
		~DataStorage()
		{
			GLuint texture = getDataStorage();
			glDeleteTextures(1, &texture);
		}
		
		std::string getType() { return "GLSLComputeEngine"; }

		void fromArray( void* array )
		{
			GLuint texture = getDataStorage();
			DataStorage::Info info = getInfo();
			GLuint format[2];

			GLSLComputeEngine::dataTypeToGLFormat( info, format );

			glBindTexture(GL_TEXTURE_2D, texture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, info.width, info.height, GL_RGBA, format[1], array);
			glBindTexture(GL_TEXTURE_2D, 0);

			glFinish();
		}

		void toArray( void* array )
		{
			GLuint texture = getDataStorage();
			DataStorage::Info info = getInfo();

			GLuint format[2];
			GLSLComputeEngine::dataTypeToGLFormat( info, format );

			//glClampColorARB( GL_CLAMP_VERTEX_COLOR_ARB, GL_FIXED_ONLY_ARB );
			glClampColorARB( GL_CLAMP_FRAGMENT_COLOR_ARB,  GL_FALSE );
			glClampColorARB( GL_CLAMP_READ_COLOR_ARB, GL_FALSE );
			//LglClampColorARB( GL_RGBA_FLOAT_MODE_ARB, GL_TRUE );

			glBindTexture(GL_TEXTURE_2D, texture);
			glGetTexImage(GL_TEXTURE_2D, 0, format[0],  format[1], array);
			glBindTexture(GL_TEXTURE_2D, 0);
			glFinish();
		}
	};

	void* bindProgram( Program* const program );
	void unbindProgram( Program* const program );

	void runProgram( Program* const program );

	vec4 reduce( const Engine::DataStorage::Ptr& storage, Engine::ReductionType type );

	Engine::DataStorage::Ptr allocateStorage( const Engine::DataStorage::Info& type, int width, int height );
	Engine::DataStorage::Ptr emptyStorage();

private:
	friend class DataStorage;

	void init();

	std::map<std::string, GLuint> loadProgram( Program* const program, std::string filepath );
	std::map<GLuint, std::map<std::string, GLuint> > uniformCache;

	bool isCached( GLuint shaderProgram );
	void cacheProgram( GLuint shaderProgram, Program* const program );
	
	static void dataTypeToGLFormat( const Engine::DataStorage::Info& type, GLuint* result );
	void flipTexture( GLuint* texture );

	static GLuint fbo_;
	static GLuint depth_;
	GLint lastFbo_;
	
	std::map<Engine::ReductionType, GLuint> reductionPrograms_;
	GLuint reduceTextures_[2];
	Engine::DataStorage::Info lastReductionTexture_;
	
	GLuint loadReduction( const std::string& filename );
	
	void saveOpenGLStateAndSetup();
	
	void restoreOpenGLState();
};

GLuint GLSLComputeEngine::fbo_ = 0;
GLuint GLSLComputeEngine::depth_ = 0;

void GLSLComputeEngine::saveOpenGLStateAndSetup()
{
	glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, &lastFbo_ );
	glPushAttrib(GL_VIEWPORT_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_PIXEL_MODE_BIT );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
	glDepthMask( GL_FALSE );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();

	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1.0f, 1.0f);
}

void GLSLComputeEngine::restoreOpenGLState()
{
	glPopAttrib();
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
	
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, lastFbo_ );
}

	void printInfoLog(GLhandleARB obj)
	{
	    int infologLength = 0;
	    int charsWritten  = 0;
	    char *infoLog;
	
	    glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB,
						 &infologLength);
	
	    if (infologLength > 0)
	    {
		infoLog = (char *)malloc(infologLength);
		glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
			printf("%s\n",infoLog);
		free(infoLog);
	    }
	}

GLuint GLSLComputeEngine::loadReduction( const std::string& name )
{
	std::string baseSource = readFile("reductions\\reduction.frag");
	std::string reductionSource = readFile( "reductions\\" + name + ".frag" );
	const char* reductionSrc = reductionSource.c_str();
	const char* baseSrc = baseSource.c_str();
	
	GLuint program = glCreateProgramObjectARB();
	GLuint baseShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	GLuint reductionShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);


	glShaderSourceARB(baseShader, 1, &baseSrc, NULL);
    glCompileShaderARB(baseShader);
    
    glShaderSourceARB(reductionShader, 1, &reductionSrc, NULL);
    glCompileShaderARB(reductionShader);
    
	printInfoLog(baseShader);
	printInfoLog(reductionShader);

    glAttachObjectARB(program, reductionShader);
    glAttachObjectARB(program, baseShader);

	glLinkProgramARB(program);
    GLint progLinkSuccess;
    glGetObjectParameterivARB(program, GL_OBJECT_LINK_STATUS_ARB,
               &progLinkSuccess);
	if (!progLinkSuccess)
	{
	    printf("Reduction %s could not be compiled.\n", name.c_str());
		printInfoLog(program);
	    exit(1);
	}
	
	return program;
}

vec4 GLSLComputeEngine::reduce( const Engine::DataStorage::Ptr& storage, Engine::ReductionType type )
{
	Engine::DataStorage::Info info = storage->getInfo();
	int width = info.width;
	int height = info.height;
	
	saveOpenGLStateAndSetup();
	glViewport(0,0,width,height);
	
	GLuint input = storage->getDataStorage();
	
	int inTex = 0;

	GLuint format[2];
	dataTypeToGLFormat( info, format );
	if( !(lastReductionTexture_ == info) )
	{	
		glBindTexture( GL_TEXTURE_2D, reduceTextures_[0] );
		glTexImage2D(GL_TEXTURE_2D, 0, format[0], width, height, 0, GL_RGBA, format[1], 0);
		glBindTexture( GL_TEXTURE_2D, reduceTextures_[1] );
		glTexImage2D(GL_TEXTURE_2D, 0, format[0], width, height, 0, GL_RGBA, format[1], 0);
	}
	
	glBindFramebufferEXT(GL_FRAMEBUFFER, fbo_);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, reduceTextures_[0], 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, reduceTextures_[1], 0);
	glBindTexture( GL_TEXTURE_2D, input );

	GLuint program = reductionPrograms_[type];
	
	GLuint xOffLocation = glGetUniformLocationARB( program, "xOff" ); 
	GLuint yOffLocation = glGetUniformLocationARB( program, "yOff" ); 
	GLuint widthLocation = glGetUniformLocationARB( program, "width" ); 
	GLuint heightLocation = glGetUniformLocationARB( program, "height" ); 
	
	glUniform1i( glGetUniformLocationARB( program, "intex" ), 0 );
	glUseProgramObjectARB( program );
	
	float dx = 1.0/width;
	float dy = 1.0/height;
	
	while( true )
	{
		int xOff = (width == 1)? 0: width - width/2;
		int yOff = (height == 1)? 0: height - height/2;
		
		width -= ( width % 2 == 0 )? width/2: (width - 1)/2;
		height -= ( height % 2 == 0 )? height/2: (height - 1)/2;
		
		glDrawBuffer( GL_COLOR_ATTACHMENT0_EXT + (inTex^1) );
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		float w = float(width)/info.width;
		float h = float(height)/info.height;
		
		glUniform1f( xOffLocation, xOff * dx );
		glUniform1f( yOffLocation, yOff * dy );
		glUniform1f( widthLocation, w);
		glUniform1f( heightLocation, h );
		
		glBegin( GL_QUADS );
			// vertex: top left
			glTexCoord2f( 0, h );
			glVertex3f( 0, h, 1 );
	
			// vertex: top right
			glTexCoord2f( w, h );
			glVertex3f( w, h, 1 );
	
			// vertex: bottom right
			glTexCoord2f( w, 0 );
			glVertex3f( w, 0, 1 );
	
			// vertex: bottom left
			glTexCoord2f( 0, 0 );
			glVertex3f( 0, 0, 1 );
		glEnd();
		
		if( width == 1 && height == 1 ) break;
		inTex = inTex^1;
		glBindTexture( GL_TEXTURE_2D, reduceTextures_[inTex] );

	}
	
	float result[4];
	
	glReadBuffer( GL_COLOR_ATTACHMENT0_EXT + (inTex^1) );
	
	glReadPixels(0,0,1,1, GL_RGBA, GL_FLOAT, result);
	
	glBindTexture( GL_TEXTURE_2D, 0 );
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, 0, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0 );
	
	glUseProgramObjectARB(0);

	restoreOpenGLState();
	
	lastReductionTexture_ = info;
	
	return vec4( result[0], result[1], result[2], result[3] );
}

void GLSLComputeEngine::flipTexture( GLuint* texture )
{
	
}

void GLSLComputeEngine::init()
{
	glGenFramebuffersEXT(1, &fbo_);
	glUniform::Initalize();
	checkGLErrors("init");
	reductionPrograms_[Engine::Minimum] = loadReduction("minimum");
	reductionPrograms_[Engine::Maximum] = loadReduction("maximum");
	reductionPrograms_[Engine::Sum] = loadReduction("sum");
	glGenTextures( 2, reduceTextures_);
	glBindTexture(GL_TEXTURE_2D, reduceTextures_[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, reduceTextures_[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void* GLSLComputeEngine::bindProgram( Program* const program )
{
	std::string engineName = this->pluginName();

	std::string uniformDeclarations = "uniform float DX;\nuniform float DY;\n";

	BOOST_FOREACH( Program::parameter_storage::value_type& param, *program )
	{
		uniformDeclarations += "uniform " + Parameter::nameFromType(param.second.type()) + " " + param.first;

		if( param.second.size() > 1 )
			uniformDeclarations += "[" + boost::lexical_cast<std::string>(param.second.size()) + "]";

		uniformDeclarations += ";\n";
	}

	if( program->getStorageInfo(Program::Input).type != DataStorage::Void )
	{
		uniformDeclarations += "uniform sampler2D intex";
		unsigned int count = program->getStorageCount(Program::Input);
		if( count > 1 )
			uniformDeclarations += "[" + boost::lexical_cast<std::string>(count) + "]";
			
		uniformDeclarations += ";\n";
	}
	
	std::string programSource;
	if( program->getProgramLocationType(engineName) == Program::File )
		programSource = readFile( program->getProgramLocationFile(engineName), true );		
	else
		programSource = std::any_cast<std::string>( program->getProgramLocationMemory(engineName) );
	// process the includes
	for( size_t includePos = programSource.find("#pragma include <");
		includePos != std::string::npos; includePos = programSource.find("#pragma include <") )
	{
		size_t left = programSource.find("<", includePos);
		size_t right = programSource.find(">", includePos);
		std::string include = programSource.substr( left + 1, right - left - 1 ) + ".frag";
		
		std::string includeSrc = readFile( "includes/" + include );
		programSource.erase( programSource.begin() + includePos, programSource.begin() + right + 1);
		programSource.insert( includePos, includeSrc );		
	}

	programSource = "#version 130\n" + uniformDeclarations + programSource + "\n";
	const char* src = programSource.c_str();
	printf("%s\n", src);
	
	GLuint* programData = new GLuint[2];

	programData[0] = glCreateProgramObjectARB();
	programData[1] = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	glShaderSourceARB(programData[1], 1, &src, NULL);
        glCompileShaderARB(programData[1]);
	//printInfoLog(programData[1]);
        glAttachObjectARB(programData[0], programData[1]);

	glLinkProgramARB(programData[0]);
        GLint progLinkSuccess;
        glGetObjectParameterivARB(programData[0], GL_OBJECT_LINK_STATUS_ARB,
               &progLinkSuccess);
	if (!progLinkSuccess)
	{
	    printf("Program %s could not be compiled.\n", program->getProgramLocationFile(engineName).c_str());
		printInfoLog(programData[1]);
		checkGLErrors("link");
	    exit(1);
	}

	cacheProgram( programData[0], program );

	return programData;
}

bool GLSLComputeEngine::isCached( GLuint shaderProgram )
{
	return uniformCache.find(shaderProgram) != uniformCache.end();
}

void GLSLComputeEngine::cacheProgram( GLuint shaderProgram, Program* const program )
{
	glUseProgramObjectARB(shaderProgram);

	BOOST_FOREACH( Program::parameter_storage::value_type& param, *program )
		uniformCache[shaderProgram][param.first] = glGetUniformLocationARB( shaderProgram, param.first.c_str() );

	if( program->getStorageInfo(Program::Input).type != DataStorage::Void )
		uniformCache[shaderProgram]["intex"] = glGetUniformLocationARB( shaderProgram, "input" );

	uniformCache[shaderProgram]["DX"] = glGetUniformLocationARB( shaderProgram, "DX" );
	uniformCache[shaderProgram]["DY"] = glGetUniformLocationARB( shaderProgram, "DY" );
	
	glUseProgramObjectARB(0);
}

void GLSLComputeEngine::unbindProgram( Program* const program )
{
	GLuint* programPtr = (GLuint*) program->getActiveProgram();
	glDeleteShader( programPtr[1] );
	glDeleteProgram( programPtr[0] );
	delete programPtr;
}

void GLSLComputeEngine::runProgram( Program* const program )
{
std::string engineName = this->pluginName();

	DataStorage::Info info = program->getStorage(Program::Output, 0)->getInfo();

	saveOpenGLStateAndSetup();
	glViewport( 0, 0, info.width, info.height );

	GLuint shaderProgram = ((GLuint*)program->getActiveProgram())[0];

	glUseProgramObjectARB( shaderProgram );

	if( !isCached(shaderProgram) )
		cacheProgram( shaderProgram, program );

	BOOST_FOREACH( Program::parameter_storage::value_type& param, *program )
		glUniform::push[param.second.type()]( uniformCache[shaderProgram][param.first], param.second );

	if( program->getStorageInfo(Program::Input).type != DataStorage::Void )
	{
		unsigned int count = program->getStorageCount(Program::Input);
		int texArray[count];
		
		for( int i = 0; i < count; i++ )
		{
			texArray[i] = i;
			glActiveTextureARB( GL_TEXTURE0_ARB + i );
			glBindTexture(GL_TEXTURE_2D, 
				std::any_cast<GLuint>(program->getStorage(Program::Input, i)->getDataStorage()));
		}
		glUniform1iv( uniformCache[shaderProgram]["intex"], count, texArray );
	}

	glUniform1f( uniformCache[shaderProgram]["DX"], 1.0f/info.width );
	glUniform1f( uniformCache[shaderProgram]["DY"], 1.0f/info.height );

	glClampColorARB( GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE );
	glClampColorARB( GL_CLAMP_FRAGMENT_COLOR_ARB,  GL_FALSE);
	glClampColorARB( GL_CLAMP_READ_COLOR_ARB, GL_FALSE );

	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, fbo_);
	
	unsigned int count = program->getStorageCount( Program::Output );
	//printf("attaching %d textures\n", count);
	GLenum renderTargets[count];
	
	for( int i = 0; i < count; i++ )
	{
		glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, 
			std::any_cast<GLuint>(program->getStorage(Program::Output, i)->getDataStorage()), 0);
		renderTargets[i] = GL_COLOR_ATTACHMENT0_EXT + i;
	}
	
	glDrawBuffers(count, renderTargets);
		
	GLenum status = glCheckFramebufferStatusEXT(GL_DRAW_FRAMEBUFFER);
	#define FRAMEBUFFER_CASE(c) case c: printf("Framebuffer error: %s\n", #c); break;
	switch( status )
	{
		FRAMEBUFFER_CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		FRAMEBUFFER_CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		FRAMEBUFFER_CASE(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
		FRAMEBUFFER_CASE(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
	}


	glBegin( GL_QUADS );
		// vertex: top left
		glTexCoord2f( 0, 1 );
		glVertex3f( 0, 1, 1 );
	
		// vertex: top right
		glTexCoord2f( 1, 1 );
		glVertex3f( 1, 1, 1 );
	
		// vertex: bottom right
		glTexCoord2f( 1, 0 );
		glVertex3f( 1, 0, 1 );
	
		// vertex: bottom left
		glTexCoord2f( 0, 0 );
		glVertex3f( 0, 0, 1 );
	glEnd();
	for( int i = 0; i < count; i++ )
		glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, 0, 0);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0);

	glUseProgramObjectARB(0);
	
	restoreOpenGLState();

	glFinish();
}

Engine::DataStorage::Ptr GLSLComputeEngine::allocateStorage( const Engine::DataStorage::Info& type, int width, int height )
{
	std::string engineName = this->pluginName();

	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	GLuint format[2];
	dataTypeToGLFormat( type, format );

	glTexImage2D(GL_TEXTURE_2D, 0, format[0], width, height, 0, GL_RGBA, format[1], 0);

	//glGenRenderbuffersEXT(1, &depth_);
	//glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, textureInfo[TEX_DEPTHBUFFER]);
	//glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
	//glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, textureInfo[TEX_DEPTHBUFFER]);

	glBindTexture(GL_TEXTURE_2D, 0);

	//GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	//if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT)
	//	printf("good\n");

	DataStorage::Info info = type;
	info.width = width;
	info.height = height;

	DataStorage* storage = new GLSLComputeEngine::DataStorage();
	storage->setDataStorage(texture);
	storage->setInfo( info );

	return GLSLComputeEngine::DataStorage::Ptr( storage );
}

Engine::DataStorage::Ptr GLSLComputeEngine::emptyStorage()
{
	return GLSLComputeEngine::DataStorage::Ptr( new GLSLComputeEngine::DataStorage() );
}

void GLSLComputeEngine::dataTypeToGLFormat( const Engine::DataStorage::Info& type, GLuint* result )
{
	if( type.type == DataStorage::Float )
	{
		result[0] = GL_RGBA32F;
		result[1] = GL_FLOAT;
		return;
	}
	if( type.type == DataStorage::Int )
	{
		result[0] = GL_RGBA16;
		result[1] = GL_INT;
		return;
	}
	if( type.type == DataStorage::Byte )
	{
		result[0] = GL_RGBA;
		result[1] = GL_UNSIGNED_BYTE;
		return;
	}
}

extern "C" 
{
	Plugin* plugin_init()
	{
		return new GLSLComputeEngine;
	}
}
