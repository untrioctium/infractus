struct InfractusProgramException: virtual std::exception, virtual boost::exception 
{
	typedef boost::error_info<struct tagErrorString,std::string> errorString;
};

class ProgramManager;
class Infractus;

/** A base class for all programs that Infractus can run. */
struct InfractusProgram
{
public:
	/** All modes of operation for a program. */
	enum ProgramMode
	{
		Static, ///< This program will consist of only a single static frame loaded from previously saved information.
		Interactive, ///< This program should expect user input.
		Dynamic, ///< This program should not expect user input.
		Playback, ///< This program will play back previously recorded information.
	};
	
	/** Empty constructor */
	InfractusProgram()
	{
	}

	virtual ~InfractusProgram()
	{
		if( bufferTexture_ != Texture() )
			Singleton<GraphicsSystem>::instance().freeTexture(bufferTexture_);
	}

	/**
	 * @brief Sets up the program for execution.
	 * @param mode The program's mode of operation.
	 * @param usingTextureSource Indicates if this program will use a texture source.
	 *        For programs without the "optional_texture" or "needs_texture" attributes,
	 *        this will always be false.
	 *        For programs with the "optional_texture" attribute, this may be true or false.
	 *        For programs with the "needs_texture" attribute, this will always be true.
	 * @param width The requested width for this program.
	 * @param height The requested height for this program.
	 *
	 * This function is only called once for any instance of an InfractusProgram.
	 * Programs 
	 */
	virtual void init( bool usingTextureSource, unsigned int width, unsigned int height ) {}
	
	/**
	 * @brief Sets the texture used for this program.
	 * @param Texture the texture that should be used.
	 * 
	 * This function will only be called is usingTextureSource was passed as true in the
	 * init() method.  In addition, it will only ever be called once.
	 */
	virtual void setUsedTexture( Texture texture ) {}
	
	/** 
	 * @brief Process user input.
	 * 
	 * This function will only be called if init() was called with interactive
	 * as true.
	 */
	virtual void input() {};
	
	/**
	 * @brief Run a frame of the program.
	 * @param dt The time elapsed since the last frame.
	 * @param scale A scale factor left to the program's interpretation.
	 *
	 * All information that will be needed in the draw() and getOutput()
	 * methods should be calculated here.
	 *
	 * Do not use the GraphicsSystem::delta() method to calculate the delta.
	 * It is not guaranteed that \a dt will be greater than zero, so
	 * make sure negative times (e.g. rewinding) are handled correctly.
	 *
	 * How to handle the scale parameter is left to the developer's imagination.
	 * Examples include scaling \a dt based on the value, increasing the size
	 * of some object, etc.  How the scale is handled, though, should be consistent
	 * throughout a program's lifetime.
	 */
	virtual void run( int dt, float scale ) {};
	
	/**
	 * @brief Save all state information needed to recreate a single frame of this program later.
	 * @return A Boost ptree containing the required information.
	 *
	 * This function is used for tagging screenshots so that they can be recreated
	 * later at varying resolutions.  The information provided in the ptree should
	 * be "just enough" to recreate a single frame of the program's state in the future.
	 * For programs that are using a texture source, information on the texture itself
	 * does not need to be saved, as this will automatically be handled by the screenshot
	 * saving system.
	 *
	 * Try to keep things as resolution independent as possible.
	 */
	//virtual ptree saveFrameInfo() { return ptree(); }
	
	/**
	 * @brief Does drawing operations on the program's buffer texture.
	 *
	 * This command is only called if the program does not have the
	 * "no_buffer" or "texture_source" attributes.
	 */
	virtual void draw() {}
	
	/**
	 * @brief Gets the output texture of this program.
	 * @return A Texture with the program's final output.
	 *
	 * The default implementation of this function returns the program's buffer
	 * texture.  If operations need to be performed on the buffer, or if another
	 * buffer besides the default buffer needs to be returned, this is the place
	 * to perform that behavior.  Do not perform any drawing operations unless
	 * you have manually bound a texture to be rendered to.
	 *
	 * Programs with the "no_buffer" attribute will need to override this in order
	 * to return their output, else a blank buffer will be returned.
	 *
	 * Programs with the "texture_source" attribute should override this as well,
	 * returning whatever texture contains the desired output.  However, any processing
	 * that needs to be done to generate this texture needs to be completely in the
	 * run() method, as there is no guarantee that this method will only be called once
	 * in a single frame.  However, it is guaraneteed that the run() method will be called
	 * before the run() method of any programs that will use this texture.
	 */
	virtual Texture getOutput() 
	{
		return bufferTexture_;
	}
	
	/**
	 * @brief Executes a command in this program.
	 * @param command The command to execute.
	 * @return The output from the command, if any.
	 *
	 * This method is only used in inspection mode.  Typically, if a program was
	 * written in a scripting language, the command would be executed in language
	 * interpreter, with the variable \c self represting the active instance.
	 */
	virtual std::string executeCommand( const std::string& command ) 
	{
		return std::string();
	}
	
	std::string getName()
	{
		return name_;
	}
	
	/** Returns this program's buffer texture. */
	Texture getBufferTexture() { return bufferTexture_; }
	
	/** Sets the buffer texture for this program. */
	void setBufferTexture( Texture bufferTexture )
	{
		bufferTexture_ = bufferTexture;
	}
	
	/** Returns the directory where this program resides. */
	std::string getWorkingDirectory() { return workingDirectory_; }
	
private:
	friend class ProgramManager;
	friend class Infractus;
	
	std::string workingDirectory_;
	Texture bufferTexture_;
	bool needsBuffer_;
	std::string name_;
	unsigned int w_,h_;
};
