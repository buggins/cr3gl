// Implementation of base platform-agnostic platform functionality.
#include "Base.h"
#include "Platform.h"
#include "Game.h"
#ifdef ENABLE_LUA
#include "ScriptController.h"
#endif

#ifdef ENABLE_FORM
#include "Form.h"
#endif

#ifndef ENABLE_FORM

/**
    * Updates all visible, enabled forms.
    */
void gameplay::Form::updateInternal(float elapsedTime) {
}


/**
    * Propagate touch events to enabled forms.
    *
    * @return Whether the touch event was consumed by a form.
    */
bool gameplay::Form::touchEventInternal(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex) {
	return false;
}


/**
    * Propagate key events to enabled forms.
    *
    * @return Whether the key event was consumed by a form.
    */
bool gameplay::Form::keyEventInternal(Keyboard::KeyEvent evt, int key) {
	return false;
}

/**
    * Propagate mouse events to enabled forms.
    *
    * @return True if the mouse event is consumed or false if it is not consumed.
    *
    * @see Mouse::MouseEvent
    */
bool gameplay::Form::mouseEventInternal(Mouse::MouseEvent evt, int x, int y, int wheelDelta) {
	return false;
}

#ifdef ENABLE_GAMEPAD
/**
    * Propagate gamepad events to enabled forms.
    *
    * @see Control::gamepadEvent
    */
static void gameplay::Form::gamepadEventInternal(Gamepad::GamepadEvent evt, Gamepad* gamepad, unsigned int analogIndex);
#endif

/**
    * Get the next highest power of two of an integer.  Used when creating framebuffers.
    *
    * @param x The number to start with.
    *
    * @return The next highest power of two after x, or x if it is already a power of two.
    */
unsigned int gameplay::Form::nextPowerOfTwo(unsigned int x) {
	return 0;
}

#endif


namespace gameplay
{

void Platform::touchEventInternal(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex, bool actuallyMouse)
{
    if (actuallyMouse || !Form::touchEventInternal(evt, x, y, contactIndex))
    {
        Game::getInstance()->touchEvent(evt, x, y, contactIndex);
#ifdef ENABLE_LUA
		Game::getInstance()->getScriptController()->touchEvent(evt, x, y, contactIndex);
#endif
    }
}

void Platform::keyEventInternal(Keyboard::KeyEvent evt, int key)
{
    if (!Form::keyEventInternal(evt, key))
    {
        Game::getInstance()->keyEvent(evt, key);
#ifdef ENABLE_LUA
        Game::getInstance()->getScriptController()->keyEvent(evt, key);
#endif
    }
}

bool Platform::mouseEventInternal(Mouse::MouseEvent evt, int x, int y, int wheelDelta)
{
    if (Form::mouseEventInternal(evt, x, y, wheelDelta))
    {
        return true;
    }
    else if (Game::getInstance()->mouseEvent(evt, x, y, wheelDelta))
    {
        return true;
    }
    else
    {
#ifdef ENABLE_LUA
        return Game::getInstance()->getScriptController()->mouseEvent(evt, x, y, wheelDelta);
#else
		return false;
#endif
    }
}

void Platform::gestureSwipeEventInternal(int x, int y, int direction)
{
    // TODO: Add support to Form for gestures
    Game::getInstance()->gestureSwipeEvent(x, y, direction);
#ifdef ENABLE_LUA
    Game::getInstance()->getScriptController()->gestureSwipeEvent(x, y, direction);
#endif
}

void Platform::gesturePinchEventInternal(int x, int y, float scale)
{
    // TODO: Add support to Form for gestures
    Game::getInstance()->gesturePinchEvent(x, y, scale);
#ifdef ENABLE_LUA
    Game::getInstance()->getScriptController()->gesturePinchEvent(x, y, scale);
#endif
}

void Platform::gestureTapEventInternal(int x, int y)
{
    // TODO: Add support to Form for gestures
    Game::getInstance()->gestureTapEvent(x, y);
#ifdef ENABLE_LUA
    Game::getInstance()->getScriptController()->gestureTapEvent(x, y);
#endif
}

void Platform::resizeEventInternal(unsigned int width, unsigned int height)
{
    // Update the width and height of the game
    Game* game = Game::getInstance();
    if (game->_width != width || game->_height != height)
    {
        game->_width = width;
        game->_height = height;
        game->resizeEvent(width, height);
#ifdef ENABLE_LUA
        game->getScriptController()->resizeEvent(width, height);
#endif
    }
}

#ifdef ENABLE_GAMEPAD
void Platform::gamepadEventInternal(Gamepad::GamepadEvent evt, Gamepad* gamepad, unsigned int analogIndex)
{
	switch(evt)
	{
	case Gamepad::CONNECTED_EVENT:
	case Gamepad::DISCONNECTED_EVENT:
		Game::getInstance()->gamepadEvent(evt, gamepad);
#ifdef ENABLE_LUA
        Game::getInstance()->getScriptController()->gamepadEvent(evt, gamepad);
#endif
		break;
	case Gamepad::BUTTON_EVENT:
	case Gamepad::JOYSTICK_EVENT:
	case Gamepad::TRIGGER_EVENT:
		Form::gamepadEventInternal(evt, gamepad, analogIndex);
		break;
	}
}

void Platform::gamepadEventConnectedInternal(GamepadHandle handle,  unsigned int buttonCount, unsigned int joystickCount, unsigned int triggerCount,
                                             unsigned int vendorId, unsigned int productId, const char* vendorString, const char* productString)
{
    Gamepad::add(handle, buttonCount, joystickCount, triggerCount, vendorId, productId, vendorString, productString);
}

void Platform::gamepadEventDisconnectedInternal(GamepadHandle handle)
{
    Gamepad::remove(handle);
}
#endif

}
