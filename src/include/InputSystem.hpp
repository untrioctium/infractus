#include <SDL2/SDL.h>

/// A small structure that refers to a coordinate on the screen.
struct ScreenCoordinate
{
	int x; ///< The x part of the coordinate.
	int y; ///< The y part of the coordinate.

	/**
	 * @brief Creates a ScreenCoordinate with the provided coordinates.
	 * @param X The x part of the coordinate.
	 * @param Y the y part of the coordinate.
	 */ 
	ScreenCoordinate( int X, int Y )
	: x(X)
	, y(Y) {}

	/** Default constructor that creates a ScreenCoordinate 
	 *  that refers to (0,0).
	 */
	ScreenCoordinate() 
	: x(0)
	, y(0) {}
};

/** Class that manages all the input for an application. */
class InputSystem
{
public:
	/** Default constructor that does nothing */
	InputSystem() {};

	/** All of the possible states for buttons or keys.*/
	enum ButtonState
	{
		Up, ///< The button is up. This is the defaul state.
		Pressed, ///< The button was pressed this frame.
		Down, ///< The button is held down.
		Released ///< The button was released this frame.
	};

	/** Sets up the input system */
	void init();

	/** 
	 * @brief Queues up pending input events, but does not actually process them.
	 *
	 * This function is important when processing the input for systems like GUIs
	 * or a console system that may need to take "priorty" over input events.
	 */
	void queueInputEvents();

	/**
	 * @brief Processes queued input events.
	 *
	 * This function must be called before using any other input functions like
	 * getKeyState(), getMouseButtonState(), getMousePosition(), getMouseMotion(),
	 * or getMouseWheelScroll().
	 * 
	 * This function will also clear the input queue, making the functions
	 * getQueuedEventsIterator(), getQueuedEventsEnd(), and removeQueuedEvent()
	 * meaningless until queueInputEvents() is called again.
	 */
	void processInputQueue();

	/** Returns true if the application was asked by the OS to quit. */
	bool quitRequested() { return quitRequested_; }
	
	/**
	 * @brief Get the current position of the mouse;
	 * @return A ScreenCoordinate that contains the position of the mouse.
	 */
	ScreenCoordinate getMousePosition() { return mousePos; }

	/**
	 * @brief Get the relative motion of the mouse;
	 * @return A ScreenCoordinate containing the change in the mouse's
	 *         x and y coordinates since the last frame.
	 */
	ScreenCoordinate getMouseMotion() { return mouseMove; }

	/**
	 * @brief Get the state of a mouse button.
	 * @param button The button to check.  1 represents the first mouse button,
	 *               2 represents the second mouse button, etc.
	 * @return The state of the button.
	 */
	ButtonState getMouseButtonState( int button ) { if( keyStates.find(-button) == keyStates.end() ) return Up; return keyStates[-button]; }
	
	/**
	 * @brief Get the amount the mouse wheel has been scrolled.
	 * @return The amount the mouse wheel has been scrolled.  Positive values indicate
	 *         forward scrolling and negative values indicate backwards scrolling.
	 */
	int getMouseWheelScroll() { return wheelScroll; }

	/**
	 * @brief Get the state of a keyboard button.
	 * @param key The key to check.
	 * @return The state of the key.
	 */
	ButtonState getKeyState( SDL_Keycode key ) {if( keyStates.find(key) == keyStates.end() ) return Up; return keyStates[key]; }

	/**
	 * @brief Returns an iterator to the beginning of the input event queue.
	 * @return A std::vector<SDL_Event>::iterator pointing to the beginning
	 *         of the input event queue.
	 */
	std::vector<SDL_Event>::iterator getQueuedEventsIterator() { return events.begin(); }

	/**
	 * @brief Returns an iterator to the end of the input event queue.
	 * @return A std::vector<SDL_Event>::iterator pointing to the end of the
	 *	   event queue.
	 */
	std::vector<SDL_Event>::iterator getQueuedEventsEnd() { return events.end(); }

	/**
	 * @brief Removes an event from the input event queue.
	 * @param i A std::vector<SDL_Event>::iterator pointing to the event to be removed.
	 * @return A std::vector<SDL_Event>::iterator pointing to the next event in the queue.
	 */
	std::vector<SDL_Event>::iterator removeQueuedEvent( std::vector<SDL_Event>::iterator i ) { return events.erase( i ); }

	/**
	 * @brief Gets the number of events currently in the input event queue.
	 * @return The size of the input event queue.
	 */
	int queuedEventCount(){ return events.size(); }

private:

	std::vector<SDL_Event> events;
	std::map<int, ButtonState> keyStates;

	int wheelScroll;
	ScreenCoordinate mousePos;
	ScreenCoordinate mouseMove;
	bool quitRequested_;

};
