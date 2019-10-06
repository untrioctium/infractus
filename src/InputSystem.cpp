#include "Global.hpp"
#include "LoggingSystem.hpp"
#include "InputSystem.hpp"

#include "Singleton.hpp"

void InputSystem::init()
{
	if( SDL_InitSubSystem( SDL_INIT_JOYSTICK ) < 0 )
	{
		Singleton<LoggingSystem>::instance().writeLog( LoggingSystem::Warning, "Cannot intialize joystick engine." );
	}

	SDL_ShowCursor(SDL_ENABLE);
	//SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL );

	Singleton<LoggingSystem>::instance().writeLog( LoggingSystem::Info, "Input system initalized." );
	quitRequested_ = false;
}

void InputSystem::queueInputEvents()
{
	events.erase( events.begin(), events.end() );

	SDL_Event e;

	mouseMove.x = 0;
	mouseMove.y = 0;	

	//std::vector<SDL_Event> pushback;

	while( SDL_PollEvent(&e) )
	{
		switch( e.type )
		{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		case SDL_MOUSEMOTION:
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEWHEEL:
		case SDL_JOYAXISMOTION:
		case SDL_JOYBALLMOTION:
		case SDL_JOYHATMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			events.push_back(e);
			break;
		case SDL_QUIT:
			quitRequested_ = true;
			break;
		//default:
		//	pushback.push_back(e);
		//	break;
		}
	}

	//for( unsigned int i = 0; i < pushback.size(); i++ )
	//{
	//	SDL_PushEvent(&pushback[i]);
	//}
}

void InputSystem::processInputQueue()
{
	std::vector<SDL_Event>::iterator e = events.begin();

	for( std::map<int, ButtonState>::iterator k = keyStates.begin();
		k != keyStates.end(); ++k )
	{
		if( k->second == Pressed )
		{
			keyStates[k->first] = Down;
		}
		else if( k->second == Released )
		{
			keyStates.erase( k++ );
			if( k == keyStates.end() ) break;
		}
		else keyStates[k->first] = k->second;
	}

	mouseMove = ScreenCoordinate();
	wheelScroll = 0;

	for( ; e != events.end(); ++e )
	{
		switch( e->type )
		{
		case SDL_KEYDOWN:
			keyStates[e->key.keysym.sym] = Pressed;
			break;

		case SDL_KEYUP:
			keyStates[e->key.keysym.sym] = Released;
			break;

		case SDL_MOUSEMOTION:
			mouseMove.x += e->motion.xrel;
			mouseMove.y += e->motion.yrel;
			mousePos.x = e->motion.x;
			mousePos.y = e->motion.y;
			break;

		case SDL_MOUSEBUTTONDOWN:
			switch(e->button.button)
			{
			case SDL_BUTTON_LEFT:
				keyStates[-1] = Pressed;
				break;

			case SDL_BUTTON_MIDDLE:
				keyStates[-3] = Pressed;
				break;

			case SDL_BUTTON_RIGHT:
				keyStates[-2] = Pressed;
				break;
			}
			break;

		case SDL_MOUSEWHEEL:
			wheelScroll += e->wheel.x + e->wheel.y;
			break;

		case SDL_MOUSEBUTTONUP:
			switch(e->button.button)
			{
			case SDL_BUTTON_LEFT:
				keyStates[-1] = Released;
				break;

			case SDL_BUTTON_MIDDLE:
				keyStates[-3] = Released;
				break;

			case SDL_BUTTON_RIGHT:
				keyStates[-2] = Released;
				break;
			}
			break;
		}
	}
}
