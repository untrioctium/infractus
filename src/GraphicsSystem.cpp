#include "Global.hpp"
#include "LoggingSystem.hpp"
//#include "FileSystem.hpp"
#include "ConfigSystem.hpp"
#include "GraphicsSystem.hpp"

#include "Singleton.hpp"

char *file2string(const char *path)
{
	FILE *fd;
	long len,
		 r;
	char *str;
 
	if (!(fd = fopen(path, "r")))
	{
		fprintf(stderr, "Can't open file '%s' for reading\n", path);
		return NULL;
	}
 
	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
 
	printf("File '%s' is %ld long\n", path, len);
 
	fseek(fd, 0, SEEK_SET);
 
	if (!(str = (char*) malloc(len * sizeof(char))))
	{
		fprintf(stderr, "Can't malloc space for '%s'\n", path);
		return NULL;
	}
 
	r = fread(str, sizeof(char), len, fd);
 
	str[r - 1] = '\0'; /* Shader sources have to term with null */
 
	fclose(fd);
 
	return str;
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

GraphicsSystem::~GraphicsSystem(void)
{ 
	SDL_Quit();
}

void GraphicsSystem::saveTexture( Texture texture, const std::string& filePath )
{
	unsigned char* texBuffer = (unsigned char*) malloc( texture.w * texture.h * 3 );
	useTexture(texture);
	glGetTexImage( GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, texBuffer );
	
	FILE* out = fopen((filePath + ".pbm").c_str(), "w");
	fprintf(out, "P3\n%d %d\n255\n", texture.w, texture.h );
	
	for( int i = 0; i < texture.w * texture.h * 3; i += 3 )
		fprintf(out, "%d %d %d\n", texBuffer[i], texBuffer[i+1], texBuffer[i+2]);
	useTexture(Texture());
	fclose(out);
	free(texBuffer);
}

void GraphicsSystem::init()
{
	Singleton<LoggingSystem>::instance().writeLog( LoggingSystem::Info, "Initializing graphics system." );

	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
	{
		Singleton<LoggingSystem>::instance().writeLogf( LoggingSystem::Critical, "Unable to initialize SDL: %s", SDL_GetError() );
		throw std::exception();
	}

	ptree config = Singleton<ConfigSystem>::instance().getConfigPtree("infractus.xml");

	try
	{
		screenInfo.w = config.get<int>("infractus.graphics.width" );
		screenInfo.h = config.get<int>("infractus.graphics.height" );
		screenInfo.d = config.get<int>("infractus.graphics.depth" );
		screenInfo.f = config.get<int>("infractus.graphics.fullscreen" );
	}
	catch(...)
	{
		Singleton<LoggingSystem>::instance().writeLog( LoggingSystem::Warning, "One or more graphics options are missing; using default values for all." );
		screenInfo.w = 800;
		screenInfo.h = 600;
		screenInfo.d = 32;
		screenInfo.f = false;
	}

	int sdl_flags = SDL_WINDOW_OPENGL;
	if( screenInfo.f ) sdl_flags |= SDL_WINDOW_FULLSCREEN;

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	
	window = SDL_CreateWindow("infractus", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenInfo.w, screenInfo.h, sdl_flags);
	SDL_GL_CreateContext(window);
	//renderer = SDL_CreateRenderer(window, -1, 0);

	if( !window )
	{
		Singleton<LoggingSystem>::instance().writeLogf( LoggingSystem::Critical, "Cannot set screen to %ix%ix%i %s: %s", screenInfo.w,
			screenInfo.h, screenInfo.d, (screenInfo.f)? "fullscreen":"windowed", SDL_GetError() );
		throw std::exception();
	}

	glewInit();

	//fontId = DT_LoadFont("images/ConsoleFont.bmp", TRANS_FONT );
	//BitFont* font = DT_FontPointer( fontId );
	//SDL_SetColorKey( font->FontSurface, SDL_SRCCOLORKEY, SDL_MapRGB( font->FontSurface->format,  255, 0, 255 ) );

	glEnable( GL_TEXTURE_2D );
 
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
 
	glViewport( 0, 0, screenInfo.w, screenInfo.h );
 
	glClear( GL_COLOR_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );

	glGenTextures( 1, &backTex );

	Singleton<LoggingSystem>::instance().writeLogf( LoggingSystem::Info, "Initialized graphics at %ix%ix%i %s %s.", screenInfo.w,
			screenInfo.h, screenInfo.d, (screenInfo.f)? "fullscreen":"windowed", glGetString(GL_VERSION) );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	lastFrameStart = SDL_GetTicks();
	lastFrameTime = 0;
	avgFrameTime = 16.0f;
	
	const char *vsSource = file2string("shaders/VertexTexture/VertexTexture.vert");
	const char *fsSource = file2string("shaders/VertexTexture/VertexTexture.frag");

	GLuint vs = glCreateShaderObjectARB(GL_VERTEX_SHADER);
	glShaderSourceARB(vs, 1, &vsSource, NULL);
	glCompileShaderARB(vs);
	printInfoLog(vs);
 
 	GLuint fs = glCreateShaderObjectARB(GL_FRAGMENT_SHADER);
	glShaderSourceARB(fs, 1, &fsSource, NULL);
	glCompileShaderARB(fs);
	printInfoLog(fs);
 
	free((void*)vsSource);
	free((void*)fsSource);
 
	latticeShader = glCreateProgramObjectARB();
	glAttachObjectARB(latticeShader, vs);
	glAttachObjectARB(latticeShader, fs);
	glLinkProgramARB(latticeShader);
	printInfoLog( latticeShader );
	
	glGenFramebuffersEXT(1, &framebuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer );
//	glGenRenderbuffersEXT(1, &depthbuffer);
	//glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthbuffer );
	//glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, screenInfo.w, screenInfo.h );
	//glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthbuffer );

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0 );
	//glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0 );
	
	glClampColorARB( GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE );
	glClampColorARB( GL_CLAMP_FRAGMENT_COLOR_ARB,  GL_FALSE);
	glClampColorARB( GL_CLAMP_READ_COLOR_ARB, GL_FALSE );
	
	blendingMap[Normal] = std::make_pair( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	blendingMap[Additive] = std::make_pair( GL_ONE, GL_ONE );
	setBlendingMode( Normal );

	
	
	//GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	//if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT)
	
	rttMode = false;
}

void GraphicsSystem::setBlendingMode( GraphicsSystem::BlendingMode mode )
{
	std::pair<GLuint, GLuint> modes = blendingMap[mode];
	glBlendFunc( modes.first, modes.second );
}

void GraphicsSystem::setDrawingMode( GraphicsSystem::DrawingMode mode )
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	if( mode == GraphicsSystem::TwoD )
		glOrtho(0, screenInfo.w, screenInfo.h, 0, -1.0f, 1.0f);
	else
		gluPerspective( 45, float(screenInfo.w)/screenInfo.h, 0.0001, 1000.0);
	
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	drawingMode = mode;
}

GraphicsSystem::DrawingMode GraphicsSystem::getDrawingMode() const
{
	return drawingMode;
}

void GraphicsSystem::pushDrawingMode()
{
	pushViewMatrix();
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	modeStack.push(drawingMode);
}

void GraphicsSystem::popDrawingMode()
{
	drawingMode = modeStack.top();
	modeStack.pop();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	popViewMatrix();
}

void GraphicsSystem::pushViewMatrix()
{
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
}

void GraphicsSystem::popViewMatrix()
{
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}

void GraphicsSystem::scaleView( Point scale )
{
	glScalef( scale.x, scale.y, scale.z );
}

void GraphicsSystem::translateView( Point trans )
{
	glTranslatef( trans.x, trans.y, trans.z );
}

void GraphicsSystem::rotateView( float angle, Point axis )
{
	glRotatef( angle, axis.x, axis.y, axis.z );
}

void GraphicsSystem::resetView()
{
	glLoadIdentity();
}

Texture GraphicsSystem::loadTexture( const std::string& filename )
{
	SDL_Surface* surface = IMG_Load( filename.c_str() );

	Texture texture;
	texture.w = surface->w;
	texture.h = surface->h;

	glGenTextures( 1, &texture.location );	

	glBindTexture( GL_TEXTURE_2D, texture.location );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		glTexImage2D( GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_RGBA_EXT,
			GL_UNSIGNED_BYTE, surface->pixels );
	#elif SDL_BYTEORDER == SDL_LIL_ENDIAN
		glTexImage2D( GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0, GL_BGRA_EXT,
			GL_UNSIGNED_BYTE, surface->pixels );
	#endif

	glBindTexture( GL_TEXTURE_2D, 0 );

	SDL_FreeSurface(surface);

	return texture;
}

void GraphicsSystem::freeTexture( Texture& texture )
{
	glDeleteTextures( 1, &texture.location );
	texture.location = 0;
}

void GraphicsSystem::drawTexture( Texture texture, Point pos, bool center, float scale, bool flipX, bool flipY )
{
	if( drawingMode == TwoD ) pos.z = getDrawDepth();
	
	float w = texture.w * scale;
	float h = texture.h * scale;

	float startX = pos.x;
	float endX = pos.x + w;
	float startY = pos.y;
	float endY = pos.y + h;

	if( center )
	{
		startX -= w/2;
		endX -= w/2;
		startY -= h/2;
		endY -= h/2;
	}

	glBindTexture( GL_TEXTURE_2D, texture.location );

	glBegin( GL_QUADS );
		//Top-left vertex (corner)
		glTexCoord2f( (flipX)? 1: 0, (flipY)? 1:0 );
		glVertex3f( startX, startY, pos.z );
	
		//Bottom-left vertex (corner)
		glTexCoord2f( (flipX)? 0: 1, (flipY)? 1:0 );
		glVertex3f( endX, startY, pos.z );
	
		//Bottom-right vertex (corner)
		glTexCoord2f( (flipX)? 0: 1, (flipY)? 0:1 );
		glVertex3f( endX, endY, pos.z );
	
		//Top-right vertex (corner)
		glTexCoord2f( (flipX)? 1: 0, (flipY)? 0:1 );
		glVertex3f( startX,endY, pos.z );
	glEnd();

	glBindTexture( GL_TEXTURE_2D, 0 );
}

void GraphicsSystem::useTexture( const Texture& texture )
{
	glBindTexture( GL_TEXTURE_2D, texture.location );
}

void GraphicsSystem::drawPointArray( const Texture& points, const Texture& colors )
{

	glUseProgramObjectARB( latticeShader );
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glBindTexture( GL_TEXTURE_2D, points.location );
	glActiveTextureARB( GL_TEXTURE0_ARB + 1 );
	glBindTexture( GL_TEXTURE_2D, colors.location );
	glUniform1iARB( glGetUniformLocationARB( latticeShader, "vertexPositions"), 0 );
	glUniform1iARB( glGetUniformLocationARB( latticeShader, "vertexColors"), 1 );
	
	Point size( points.w, points.h );
	//glPointSize(3);
	
	if( pointArrayCache.count(size) == 0 )
	{
		float yStep = 1/float(points.w);
		float xStep = 1/float(points.h);
		GLuint cache = glGenLists(1);
		
		glNewList(cache, GL_COMPILE);
			glBegin(GL_POINTS);
				for( float y = yStep/2; y < 1; y+=yStep )
					for( float x = xStep/2; x < 1; x+=xStep )
						glVertex3f( x, y, 0 );
			glEnd();
		glEndList();
		
		pointArrayCache[size] = cache;
	}
	
	glDisable(GL_DEPTH_TEST);
				
	glCallList( pointArrayCache[size] );
	
	glEnable(GL_DEPTH_TEST);
	
	glUseProgramObjectARB(0);
	glBindTexture( GL_TEXTURE_2D, 0 );
	glActiveTextureARB( GL_TEXTURE0_ARB );
	glBindTexture( GL_TEXTURE_2D, 0 );
}

Texture GraphicsSystem::createBufferTexture( unsigned int width, unsigned int height )
{
	if( width == 0 ) width = screenInfo.w;
	if( height == 0 ) height = screenInfo.h;
	
	Texture bufferTexture;
	bufferTexture.w = width;
	bufferTexture.h = height;
	glGenTextures( 1, &bufferTexture.location );
	
	glBindTexture( GL_TEXTURE_2D, bufferTexture.location );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA,
			GL_FLOAT, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	return bufferTexture;
}

void GraphicsSystem::drawToTexture( Texture texture )
{
	if( texture == Texture() )
	{
		rttMode = false;
		glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, 0, 0);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0 );
		glDrawBuffer(GL_BACK_LEFT);
		glViewport(0,0,screenInfo.w, screenInfo.h);
	//	drawnItems = oldDrawnItems;
		return;
	}
	
	rttMode = true;
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, framebuffer );
	glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture.location, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glViewport(0,0,texture.w, texture.h);
	//glClear(GL_DEPTH_BUFFER_BIT);
	//oldDrawnItems = drawnItems;
	//drawnItems = 0;
}

int GraphicsSystem::getFontHeight() const { return 0;}//DT_FontPointer( fontId )->CharHeight; }
int GraphicsSystem::getFontWidth() const { return 0;}//DT_FontPointer( fontId )->CharWidth; }

void GraphicsSystem::drawText( const std::string& text, Point pos )
{
	/*BitFont* font = DT_FontPointer( fontId );
	SDL_Surface* tempSurface = requestBufferSurface( font->CharWidth * text.size(), font->CharHeight );
	SDL_FillRect( tempSurface, NULL, SDL_MapRGBA( tempSurface->format, 0, 0, 0, 0 ) );
	DT_DrawText( text.c_str(), tempSurface, fontId, 0, 0 );
	SDL_UpdateRect( tempSurface, 0, 0, 0, 0 );

	drawSurface( tempSurface, pos );
	SDL_FreeSurface( tempSurface );*/
}

void GraphicsSystem::resetDrawColor()
{
	glColor4f( 1.0, 1.0, 1.0, 1.0 );
}

void GraphicsSystem::setDrawColor( Color color )
{
	glColor4f( color.r, color.g, color.b, color.a );
}

void GraphicsSystem::clearToColor( Color color )
{
	glClearColor( color.r, color.g, color.b, color.a );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT );
}

void GraphicsSystem::drawRectangle( Point ul, Point br )
{
	if( drawingMode == TwoD && ul.z < -1 && br.z < -1 ) 
	{
		float depth = getDrawDepth();
		ul.z = depth;
		br.z = depth;
	}
	glBegin(GL_QUADS);
		// upper left
		glTexCoord2f( 0, 0 );
		glVertex3f(ul.x, ul.y, ul.z);
		
		// upper right
		glTexCoord2f( 1, 0 );
		glVertex3f(br.x, ul.y, br.z);
		
		// bottom right
		glTexCoord2f( 1, 1 );
		glVertex3f(br.x, br.y, br.z);
		
		// bottom left
		glTexCoord2f( 0, 1 );
		glVertex3f(ul.x, br.y, ul.z);
	glEnd();
}

void GraphicsSystem::drawLine( Point start, Point end )
{
	if( drawingMode == TwoD ) 
	{
		float depth = getDrawDepth();
		start.z = depth;
		end.z = depth;
	}
	glBegin(GL_LINES);
		glVertex3f(start.x, start.y, start.z);
		glVertex3f(end.x, end.y, end.z);
	glEnd();
}

void GraphicsSystem::setPointSize( float size )
{
	glPointSize(size);
}

void GraphicsSystem::drawPoint( Point pos )
{
	if( drawingMode == TwoD ) pos.z = getDrawDepth();
	
	glBegin(GL_POINTS);
		glVertex3f(pos.x, pos.y, pos.z);
	glEnd();
}

void GraphicsSystem::setWindowTitle( const std::string& title )
{
	//if( !screenInfo.f )
	//	SDL_WM_SetCaption(title.c_str(), NULL );
}

void GraphicsSystem::drawSurface( SDL_Surface* texture, Point pos, bool center, float scale )
{
	if( texture == NULL ) return;
	if( drawingMode == TwoD && pos.z < -1 ) pos.z = getDrawDepth();
	
	SDL_Surface* temp;

	//if( texture->flags & SDL_SRCCOLORKEY )
	//{
	//	temp = requestBufferSurface( texture->w, texture->h );
	//	SDL_FillRect( temp, NULL, SDL_MapRGBA( temp->format, 0, 0, 0, 0 ));
	//	SDL_BlitSurface( texture, NULL, temp, NULL );
	//} else temp = texture;
	temp = texture;

	glBindTexture( GL_TEXTURE_2D, backTex );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		glTexImage2D( GL_TEXTURE_2D, 0, 4, temp->w, temp->h, 0, GL_RGBA,
			GL_UNSIGNED_BYTE, temp->pixels );
	#elif SDL_BYTEORDER == SDL_LIL_ENDIAN
		glTexImage2D( GL_TEXTURE_2D, 0, 4, temp->w, temp->h, 0, GL_BGRA,
			GL_UNSIGNED_BYTE, temp->pixels );
	#endif
	
	float w = temp->w * scale;
	float h = temp->h * scale;
	float startX = pos.x;
	float endX = pos.x + w;
	float startY = pos.y;
	float endY = pos.y + h;

	if( center )
	{
		startX -= w/2;
		endX -= w/2;
		startY -= h/2;
		endY -= h/2;
	}
	
	glBegin( GL_QUADS );
		//Top-left vertex (corner)
		glTexCoord2f( 0, 0 );
		glVertex3f( startX, startY, pos.z );
	
		//Bottom-left vertex (corner)
		glTexCoord2f( 1, 0 );
		glVertex3f( endX, startY, pos.z );
	
		//Bottom-right vertex (corner)
		glTexCoord2f( 1, 1 );
		glVertex3f( endX, endY, pos.z );
	
		//Top-right vertex (corner)
		glTexCoord2f( 0, 1 );
		glVertex3f( startX,endY, pos.z );
	glEnd();

	glBindTexture( GL_TEXTURE_2D, 0 );
	
	//if( texture->flags & SDL_SRCCOLORKEY )
	//	SDL_FreeSurface(temp);
}

void GraphicsSystem::render()
{
	//if( gui ) GuiSystem::getSingleton()->render();

	if( rttMode )
	{
		printf("Warning: trying to render in draw to texture mode.\n");
	}

	resetDrawColor();
	SDL_GL_SwapWindow(window);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
	
	SDL_Delay(0);
	drawnItems = 0;
	
	Uint32 ticks = SDL_GetTicks();
	lastFrameTime = ticks - lastFrameStart;
	lastFrameStart = ticks;
	
	avgFrameTime = FRAME_DELTA_WEIGHT * lastFrameTime + (1 - FRAME_DELTA_WEIGHT) * avgFrameTime;
}

SDL_Surface* GraphicsSystem::requestBufferSurface(int w, int h)
{
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		return SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, screenInfo.d, 0xff000000, 
			0x00ff0000, 0x0000ff00, 0x000000ff);
	#else
		return 0; //SDL_CreateRGBSurface(SDL_HWSURFACE, w, h, screenInfo.d, 0x000000ff, 
			//0x0000ff00, 0x00ff0000, 0xff000000);
	#endif
}

float GraphicsSystem::getDrawDepth()
{
	static const float DEPTH_FACTOR = 8.0f;
	drawnItems++;
	return (drawnItems - DEPTH_FACTOR)/(drawnItems + DEPTH_FACTOR);
}
