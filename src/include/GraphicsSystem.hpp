#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
/// A small struct that refers to a (x,y,z) point in space.
struct Point
{
	float x; ///< The x coordinate of the point.
	float y; ///< The y coordinate of the point.
	float z; ///< The z coordinate of the point.
	
	Point( float X, float Y, float Z )
	: x(X)
	, y(Y)
	, z(Z) {}
	
	Point( float X, float Y )
	: x(X)
	, y(Y) 
	, z(-2) {}
	
	bool operator<( const Point& other ) const
	{
		if( x < other.x ) return true;
		if( y < other.y ) return true;
		if( z < other.z ) return true;
		return false;
	}
};

/// A small struct that refers to a color.
struct Color
{
	float r; ///< The color's red value in the range [0,1].
	float g; ///< The color's green value in the range [0,1].
	float b; ///< The color's blue value in the range [0,1].
	float a; ///< The color's alpha value in the range [0,1].
	
	Color( float R, float G, float B, float A )
	: r(R)
	, g(G)
	, b(B)
	, a(A) {}
	
	Color( float R, float G, float B )
	: r(R)
	, g(G)
	, b(B)
	, a(1.0) {}
};

/// A small struct that holds the screen dimensions and depth.
struct ScreenInfo
{
	int w; ///< The screen's current width
	int h; ///< The screen's current height
	int d; ///< The screen's current color depth. Right now, I don't think I support anything other than 32bpp.
	bool f; ///< True if the application is fullscreen.
};

/// A small structure that holds information on an OpenGL Texture.
struct Texture
{
	GLuint location; ///< OpenGL internal indentifier that helps it locate the texture in video memory.
	int w; ///< The width of the texture.
	int h; ///< The height of the texture.
	
	Texture()
	: location(0)
	, w(0)
	, h(0) {}
	
	bool operator==( const Texture& other ) const
	{
		return location == other.location;
	}
	
	bool operator!=( const Texture& other ) const
	{
		return location != other.location;
	}
};

/** A class that manages the graphics for the application via OpenGL and SDL. */
class GraphicsSystem
{
public:

	/** The type of storage used to store an array of points. */
	typedef std::vector<Point> PointStorage;

	/** Defines the selectable drawing modes. */
	enum DrawingMode
	{
		TwoD, ///< All drawing is done in 2D.
		ThreeD, ///< All drawing is done in 3D.
	};
	
	/** Defines blending modes. */
	enum BlendingMode
	{
		Normal, ///< Blending is done normally.
		Additive,
	};

	/** Empty default constructor */
	GraphicsSystem() {}
	
	/** Closes the graphics context and frees any Textures or textures that were loaded. */
	~GraphicsSystem();

	/** Starts up the graphics system. */
	void init();

	/**
	 * @brief Sets the drawing mode.
	 * @param mode A DrawingMode representing the mode to set to.
	 */
	void setDrawingMode( DrawingMode mode );
	
	/** Gets the current drawing mode. */
	DrawingMode getDrawingMode() const;
	
	/** Pushes the current drawing mode on to the stack. */
	void pushDrawingMode();
	
	/** Restores the drawing mode to the one on the top of the stack. */
	void popDrawingMode();
	
	/** Pushes the current view matrix on to the stack. */
	void pushViewMatrix();
	
	/** Restores the view matrix to the one on the top of the stack. */
	void popViewMatrix();
	
	/**
	 * @brief Scales the current view matrix by a factor.
	 * @param scale The factors to scale each axis by.
	 */
	void scaleView( Point scale );
	
	/**
	 * @brief Translates the current view matrix.
	 * @translate The amount to translate in each direction.
	 */
	void translateView( Point translate );
	
	/**
	 * @brief Rotates the current view matrix.
	 * @param angle The angle to rotate by.
	 * @param axis The axis to rotate around.
	 */
	void rotateView( float angle, Point axis );

	/** Resets the current view matrix to the default. */
	void resetView();

	void setBlendingMode( BlendingMode mode );

	/**
	 * @brief Gets information about the screen.
	 * @return A ScreenInfo struct with the screen's information.
	 */
	ScreenInfo getScreenInfo() const { return screenInfo; }

	/**
	 * @brief Loads a Texture into OpenGL's video memory.
	 * @param filename A path to the image file that will be loaded.
	 * @return A Texture structure representing the loaded image.
	 *
	 * The only image formats currently supported are TGA and BMP.
	 */
	Texture loadTexture( const std::string& filename );

	void freeTexture( Texture& texture );

	/**
	 * @brief Draws a Texture to the screen.
	 * @param Texture The Texture to draw.
	 * @param pos The point to draw at.
	 * @param center If true, the Texture will be drawn so that its center is on (x,y).
	 * @param flipX If true, the Texture will be flipped in the X direction.
	 * @param flipY If true, the Texture will be flipped in the Y direction.
	 * @param scale The scale factor of the Texture.
	 */
	void drawTexture( Texture texture, Point pos, 
		bool center = false, float scale = 1.0f, bool flipX = false, bool flipY = false);

	/** 
	 * @brief Sets the texture to be used for future drawing operations.
	 * @param The texture to use.  The default value of Texture() clears the active texture (if there is one).
	 */
	void useTexture( const Texture& texture = Texture() );

	/**
	 * @brief Clears the entire screen to a color.
	 * @param color The color to clear to.
	 */
	void clearToColor( Color color );
	
	/** Gets the frames per second count. */
	float getFPS() const { return 1000.0/avgFrameTime; }
	
	/** Returns the last frame's excecution time in milliseconds. */
	Uint32 delta() const { return lastFrameTime; }
	
	/**
	 * @brief Draws a filled rectangle on the screen.
	 * @param ul The point defining the rectangle's upper left coordinate.
	 * @param br The point defining the rectangle's bottom right coordinate.
	 */
	void drawRectangle( Point ul, Point br );
	
	/**
	 * @brief Draws a line on the screen.
	 * @param start The starting point for the line.
	 * @param end The ending point for the line.
	 */
	void drawLine( Point start, Point end );
	
	/**
	 * @brief Draws a curve to the screen.
	 * @param curve A PointStorage containing all the points that make up the curve.
	 */
	void drawCurve( const PointStorage& curve );
	
	void setPointSize( float size );
	
	/**
	 * @brief Draws a point on the screen.
	 * @param pos The position to draw the point.
	 */
	void drawPoint( Point pos );

	/**
	 * @brief Draws multiple points to the screen.
	 * @param points A PointStorage containing all the points to draw.
	 */
	void drawPoints( const PointStorage& points );
	
	/**
	 * @brief Draws an array of points from source textures.
	 * @param points A Texture where each pixel contains a position of a point.
	 * @param colors A Texture where each pixel contains each point's color.
	 */
	void drawPointArray( const Texture& points, const Texture& colors );
	
	/**
	 * @brief Enables drawing to a texture.
	 * @param texture The texture that will be drawn to. The default value of Texture() disables drawing to texture.
	 */
	void drawToTexture( Texture texture = Texture() );

	/**
	 * @brief Sets the color for all future drawing operations.
	 * @param color The color to use.
	 *
	 * This operation affects all drawing operations, including drawTexture() and
	 * drawSurface().  Unless you want to to drawing with a specific tint, call
	 * resetDrawColor() before calling these functions.
	 */
	void setDrawColor( Color color );

	/** Resets the drawing color to its default value of white. */
	void resetDrawColor();

	/**
	 * @brief Sets the window's title.
	 * @param title The title to set.
	 *
	 * This function will have no effect if the application is fullscreen.
	 */
	 void setWindowTitle( const std::string& title );
	 
	/**
	 * @brief Sets the window's icon.
	 * @param icon The icon to use.
	 *
	 * This function will have no effect if the application is fullscreen.
	 */
	 void setWindowIcon( SDL_Surface* icon );

	/**
	 * @brief Draw a SDL_Surface to the screen.
	 * @param surface The surface to draw.
	 * @param Pos The coordinate to draw at.
	 * @param center If true, the surface will be drawn so that its center is on (x,y).
	 *
	 * This operation has to copy the surface to video memory, which is slower than
	 * using a Texture.
	 */
	void drawSurface( SDL_Surface* surface, Point pos, bool center = false, float scale = 1.0f );

	/** Gets the height in pixels of the current font */
	int getFontHeight() const;
	
	/** Gets the width in pixels of a single character in current font */
	int getFontWidth() const;

	/**
	 * @brief Draws a string of text to the screen.
	 * @param text The text to draw.
	 * @param pos The position to draw the text.
	 */
	void drawText( const std::string& text, Point pos );

	/** Renders the current frame to the screen. */
	void render();

	Texture createBufferTexture( unsigned int width = 0, unsigned int height = 0 );

	/**
	 * @brief Returns a SDL_Surface in the screen's format.
	 * @param w The requested width of the surface.
	 * @param h The requested height of the surface.
	 * @return A pointer to the allocated SDL_Surface.
	 *
	 * Unlike a Texture, the GraphicsSystem does not take ownership of the
	 * returned SDL_Surface, and therefore it must be freed manually using
	 * a call to SDL_FreeSurface().
	 */
	SDL_Surface* requestBufferSurface(int w, int h);

	void saveTexture( Texture texture, const std::string& filePath );

private:

	float getDrawDepth();
	int drawnItems;
	int oldDrawnItems;
	
	int fontId;
	
	GLuint latticeShader;
	
	Uint32 lastFrameStart;
	Uint32 lastFrameTime;
	float avgFrameTime;
	static constexpr float FRAME_DELTA_WEIGHT = .2;
	
	GLuint framebuffer;
	GLuint depthbuffer;
	bool rttMode;

	ScreenInfo screenInfo;
	DrawingMode drawingMode;
	std::stack<DrawingMode> modeStack;
	
	std::map<Point, GLuint> pointArrayCache;
	std::map<BlendingMode, std::pair<GLuint, GLuint> > blendingMap;

	SDL_Window* window;
	SDL_Renderer* renderer;

	GLuint backTex;
};
