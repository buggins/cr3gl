#include "Base.h"
#include "Game.h"
#include "Platform.h"
#include "FileSystem.h"
#ifdef ENABLE_UI
#include "RenderState.h"
#include "FrameBuffer.h"
#include "SceneLoader.h"
#endif

/** @script{ignore} */
GLenum __gl_error_code = GL_NO_ERROR;
#ifdef ENABLE_AUDIO
/** @script{ignore} */
ALenum __al_error_code = AL_NO_ERROR;
#endif

namespace gameplay
{

static Game* __gameInstance = NULL;
double Game::_pausedTimeLast = 0.0;
double Game::_pausedTimeTotal = 0.0;

Game::Game()
    : _initialized(false), _state(UNINITIALIZED), _pausedCount(0),
      _frameLastFPS(0), _frameCount(0), _frameRate(0),
      _clearDepth(1.0f), _clearStencil(0), _properties(NULL),
#ifdef ENABLE_ANIMATION
      _animationController(NULL), 
#endif
#ifdef ENABLE_AUDIO
	  _audioController(NULL),
#endif
#ifdef ENABLE_PHYSICS
      _physicsController(NULL), 
#endif
#ifdef ENABLE_AI
	  _aiController(NULL), 
#endif
#ifdef ENABLE_AUDIO
	  _audioListener(NULL),
#endif
      _timeEvents(NULL)
#ifdef ENABLE_LUA
	  ,
	  _scriptController(NULL), 
	  _scriptListeners(NULL)
#endif
{
    GP_ASSERT(__gameInstance == NULL);
    __gameInstance = this;
    _timeEvents = new std::priority_queue<TimeEvent, std::vector<TimeEvent>, std::less<TimeEvent> >();
}

Game::~Game()
{
#ifdef ENABLE_LUA
	SAFE_DELETE(_scriptController);
#endif

    // Do not call any virtual functions from the destructor.
    // Finalization is done from outside this class.
    SAFE_DELETE(_timeEvents);
#ifdef GAMEPLAY_MEM_LEAK_DETECTION
    Ref::printLeaks();
    printMemoryLeaks();
#endif
}

Game* Game::getInstance()
{
    GP_ASSERT(__gameInstance);
    return __gameInstance;
}

double Game::getAbsoluteTime()
{
    return Platform::getAbsoluteTime();
}

double Game::getGameTime()
{
    return Platform::getAbsoluteTime() - _pausedTimeTotal;
}

void Game::setVsync(bool enable)
{
    Platform::setVsync(enable);
}

bool Game::isVsync()
{
    return Platform::isVsync();
}

int Game::run()
{
    if (_state != UNINITIALIZED)
        return -1;

    loadConfig();

    _width = Platform::getDisplayWidth();
    _height = Platform::getDisplayHeight();

    // Start up game systems.
    if (!startup())
    {
        shutdown();
        return -2;
    }

    return 0;
}

bool Game::startup()
{
    if (_state != UNINITIALIZED)
        return false;

    setViewport(Rectangle(0.0f, 0.0f, (float)_width, (float)_height));
#ifdef ENABLE_UI
    RenderState::initialize();
    FrameBuffer::initialize();
#endif

#ifdef ENABLE_ANIMATION
	_animationController = new AnimationController();
    _animationController->initialize();
#endif

#ifdef ENABLE_AUDIO
    _audioController = new AudioController();
    _audioController->initialize();
#endif
#ifdef ENABLE_PHYSICS
    _physicsController = new PhysicsController();
    _physicsController->initialize();
#endif

#ifdef ENABLE_AI
    _aiController = new AIController();
    _aiController->initialize();
#endif

#ifdef ENABLE_LUA
    _scriptController = new ScriptController();
    _scriptController->initialize();
#endif

    // Load any gamepads, ui or physical.
    loadGamepads();

    // Set the script callback functions.
    if (_properties)
    {
#ifdef ENABLE_LUA
        Properties* scripts = _properties->getNamespace("scripts", true);
        if (scripts)
        {
            const char* callback;
            while ((callback = scripts->getNextProperty()) != NULL)
            {
                std::string url = scripts->getString();
                std::string file;
                std::string id;
                splitURL(url, &file, &id);

                if (file.size() <= 0 || id.size() <= 0)
                {
                    GP_ERROR("Invalid %s script callback function '%s'.", callback, url.c_str());
                }
                else
                {
                    _scriptController->loadScript(file.c_str());
                    _scriptController->registerCallback(callback, id.c_str());
                }
            }
        }
#endif
    }

    _state = RUNNING;

    return true;
}

void Game::shutdown()
{
    // Call user finalization.
    if (_state != UNINITIALIZED)
    {
#ifdef ENABLE_ANIMATION
        GP_ASSERT(_animationController);
#endif
#ifdef ENABLE_AUDIO
        GP_ASSERT(_audioController);
#endif
#ifdef ENABLE_PHYSICS
        GP_ASSERT(_physicsController);
#endif
#ifdef ENABLE_AI
        GP_ASSERT(_aiController);
#endif

        Platform::signalShutdown();

		// Call user finalize
        finalize();

		// Shutdown scripting system first so that any objects allocated in script are released before our subsystems are released
#ifdef ENABLE_LUA
		_scriptController->finalizeGame();
		if (_scriptListeners)
		{
			for (size_t i = 0; i < _scriptListeners->size(); i++)
			{
				SAFE_DELETE((*_scriptListeners)[i]);
			}
			SAFE_DELETE(_scriptListeners);
		}
		_scriptController->finalize();
#endif

#ifdef ENABLE_GAMEPAD
		unsigned int gamepadCount = Gamepad::getGamepadCount();
        for (unsigned int i = 0; i < gamepadCount; i++)
        {
            Gamepad* gamepad = Gamepad::getGamepad(i, false);
            SAFE_DELETE(gamepad);
        }
#endif
#ifdef ENABLE_ANIMATION
        _animationController->finalize();
        SAFE_DELETE(_animationController);
#endif
#ifdef ENABLE_AUDIO
        _audioController->finalize();
        SAFE_DELETE(_audioController);
#endif

#ifdef ENABLE_PHYSICS
        _physicsController->finalize();
        SAFE_DELETE(_physicsController);
#endif
#ifdef ENABLE_AI
        _aiController->finalize();
        SAFE_DELETE(_aiController);
#endif

        // Note: we do not clean up the script controller here
        // because users can call Game::exit() from a script.

#ifdef ENABLE_AUDIO
        SAFE_DELETE(_audioListener);
#endif

#ifdef ENABLE_UI
        FrameBuffer::finalize();
        RenderState::finalize();
#endif
        SAFE_DELETE(_properties);

		_state = UNINITIALIZED;
    }
}

void Game::pause()
{
    if (_state == RUNNING)
    {
#ifdef ENABLE_ANIMATION
		GP_ASSERT(_animationController);
#endif
#ifdef ENABLE_AUDIO
        GP_ASSERT(_audioController);
#endif
#ifdef ENABLE_PHYSICS
        GP_ASSERT(_physicsController);
#endif
#ifdef ENABLE_AI
        GP_ASSERT(_aiController);
#endif
        _state = PAUSED;
        _pausedTimeLast = Platform::getAbsoluteTime();
#ifdef ENABLE_ANIMATION
		_animationController->pause();
#endif
#ifdef ENABLE_AUDIO
        _audioController->pause();
#endif
#ifdef ENABLE_PHYSICS
        _physicsController->pause();
#endif
#ifdef ENABLE_AI
        _aiController->pause();
#endif
    }

    ++_pausedCount;
}

void Game::resume()
{
    if (_state == PAUSED)
    {
        --_pausedCount;

        if (_pausedCount == 0)
        {
#ifdef ENABLE_ANIMATION
			GP_ASSERT(_animationController);
#endif
#ifdef ENABLE_AUDIO
            GP_ASSERT(_audioController);
#endif
#ifdef ENABLE_PHYSICS
            GP_ASSERT(_physicsController);
#endif
#ifdef ENABLE_AI
            GP_ASSERT(_aiController);
#endif
            _state = RUNNING;
            _pausedTimeTotal += Platform::getAbsoluteTime() - _pausedTimeLast;
#ifdef ENABLE_ANIMATION
			_animationController->resume();
#endif
#ifdef ENABLE_AUDIO
            _audioController->resume();
#endif
#ifdef ENABLE_PHYSICS
            _physicsController->resume();
#endif
#ifdef ENABLE_AI
            _aiController->resume();
#endif
        }
    }
}

void Game::exit()
{
    // Only perform a full/clean shutdown if FORCE_CLEAN_SHUTDOWN or
    // GAMEPLAY_MEM_LEAK_DETECTION is defined. Every modern OS is able to
    // handle reclaiming process memory hundreds of times faster than it
    // would take us to go through every pointer in the engine and release
    // them nicely. For large games, shutdown can end up taking long time,
    // so we'll just call ::exit(0) to force an instant shutdown.

#if defined FORCE_CLEAN_SHUTDOWN || defined GAMEPLAY_MEM_LEAK_DETECTION

    // Schedule a call to shutdown rather than calling it right away.
	// This handles the case of shutting down the script system from
	// within a script function (which can cause errors).
	static ShutdownListener listener;
	schedule(0, &listener);

#else

    // End the process immediately without a full shutdown
    ::exit(0);

#endif
}


void Game::frame()
{
    if (!_initialized)
    {
        // Perform lazy first time initialization
        initialize();
#ifdef ENABLE_LUA
        _scriptController->initializeGame();
#endif
        _initialized = true;

        // Fire first game resize event
        Platform::resizeEventInternal(_width, _height);
    }

	static double lastFrameTime = Game::getGameTime();
	double frameTime = getGameTime();

    // Fire time events to scheduled TimeListeners
    fireTimeEvents(frameTime);

    if (_state == Game::RUNNING)
    {
#ifdef ENABLE_ANIMATION
		GP_ASSERT(_animationController);
#endif
#ifdef ENABLE_AUDIO
        GP_ASSERT(_audioController);
#endif
#ifdef ENABLE_PHYSICS
        GP_ASSERT(_physicsController);
#endif
#ifdef ENABLE_AI
        GP_ASSERT(_aiController);
#endif

        // Update Time.
        float elapsedTime = (frameTime - lastFrameTime);
        lastFrameTime = frameTime;

#ifdef ENABLE_ANIMATION
        // Update the scheduled and running animations.
        _animationController->update(elapsedTime);
#endif
#ifdef ENABLE_PHYSICS
        // Update the physics.
        _physicsController->update(elapsedTime);
#endif

#ifdef ENABLE_AI
        // Update AI.
        _aiController->update(elapsedTime);
#endif

#ifdef ENABLE_GAMEPAD
		// Update gamepads.
        Gamepad::updateInternal(elapsedTime);
#endif

        // Application Update.
        update(elapsedTime);

		// Update forms.
        Form::updateInternal(elapsedTime);

        // Run script update.
#ifdef ENABLE_LUA
        _scriptController->update(elapsedTime);
#endif

        // Audio Rendering.
#ifdef ENABLE_AUDIO
        _audioController->update(elapsedTime);
#endif

        // Graphics Rendering.
        render(elapsedTime);

#ifdef ENABLE_LUA
        // Run script render.
        _scriptController->render(elapsedTime);
#endif

        // Update FPS.
        ++_frameCount;
        if ((Game::getGameTime() - _frameLastFPS) >= 1000)
        {
            _frameRate = _frameCount;
            _frameCount = 0;
            _frameLastFPS = getGameTime();
        }
    }
	else if (_state == Game::PAUSED)
    {
#ifdef ENABLE_GAMEPAD
		// Update gamepads.
        Gamepad::updateInternal(0);
#endif

        // Application Update.
        update(0);

        // Update forms.
        Form::updateInternal(0);

        // Script update.
#ifdef ENABLE_LUA
        _scriptController->update(0);
#endif

        // Graphics Rendering.
        render(0);

        // Script render.
#ifdef ENABLE_LUA
        _scriptController->render(0);
#endif
    }
}

void Game::renderOnce(const char* function)
{
#ifdef ENABLE_LUA
    _scriptController->executeFunction<void>(function, NULL);
#endif
    Platform::swapBuffers();
}

void Game::updateOnce()
{
#ifdef ENABLE_ANIMATION
	GP_ASSERT(_animationController);
#endif
#ifdef ENABLE_AUDIO
    GP_ASSERT(_audioController);
#endif
#ifdef ENABLE_PHYSICS
    GP_ASSERT(_physicsController);
#endif
#ifdef ENABLE_AI
    GP_ASSERT(_aiController);
#endif

    // Update Time.
    static double lastFrameTime = getGameTime();
    double frameTime = getGameTime();
    float elapsedTime = (frameTime - lastFrameTime);
    lastFrameTime = frameTime;

#ifdef ENABLE_ANIMATION
    // Update the internal controllers.
    _animationController->update(elapsedTime);
#endif
#ifdef ENABLE_PHYSICS
    _physicsController->update(elapsedTime);
#endif
#ifdef ENABLE_AI
    _aiController->update(elapsedTime);
#endif
#ifdef ENABLE_AUDIO
    _audioController->update(elapsedTime);
#endif
#ifdef ENABLE_LUA
    _scriptController->update(elapsedTime);
#endif
}

void Game::setViewport(const Rectangle& viewport)
{
    _viewport = viewport;
    glViewport((GLuint)viewport.x, (GLuint)viewport.y, (GLuint)viewport.width, (GLuint)viewport.height);
}

void Game::clear(ClearFlags flags, const Vector4& clearColor, float clearDepth, int clearStencil)
{
    GLbitfield bits = 0;
    if (flags & CLEAR_COLOR)
    {
        if (clearColor.x != _clearColor.x ||
            clearColor.y != _clearColor.y ||
            clearColor.z != _clearColor.z ||
            clearColor.w != _clearColor.w )
        {
            glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
            _clearColor.set(clearColor);
        }
        bits |= GL_COLOR_BUFFER_BIT;
    }

    if (flags & CLEAR_DEPTH)
    {
        if (clearDepth != _clearDepth)
        {
            glClearDepth(clearDepth);
            _clearDepth = clearDepth;
        }
        bits |= GL_DEPTH_BUFFER_BIT;

#ifdef ENABLE_UI
        // We need to explicitly call the static enableDepthWrite() method on StateBlock
        // to ensure depth writing is enabled before clearing the depth buffer (and to
        // update the global StateBlock render state to reflect this).
        RenderState::StateBlock::enableDepthWrite();
#endif
    }

    if (flags & CLEAR_STENCIL)
    {
        if (clearStencil != _clearStencil)
        {
            glClearStencil(clearStencil);
            _clearStencil = clearStencil;
        }
        bits |= GL_STENCIL_BUFFER_BIT;
    }
    glClear(bits);
}

void Game::clear(ClearFlags flags, float red, float green, float blue, float alpha, float clearDepth, int clearStencil)
{
    clear(flags, Vector4(red, green, blue, alpha), clearDepth, clearStencil);
}

#ifdef ENABLE_AUDIO
AudioListener* Game::getAudioListener()
{
    if (_audioListener == NULL)
    {
        _audioListener = new AudioListener();
    }
    return _audioListener;
}
#endif

void Game::menuEvent()
{
}

void Game::keyEvent(Keyboard::KeyEvent evt, int key)
{
}

void Game::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
}

bool Game::mouseEvent(Mouse::MouseEvent evt, int x, int y, int wheelDelta)
{
    return false;
}

void Game::resizeEvent(unsigned int width, unsigned int height)
{
	_width = width;
	_height = height;
    setViewport(Rectangle(0.0f, 0.0f, (float)width, (float)height));
}

bool Game::isGestureSupported(Gesture::GestureEvent evt)
{
    return Platform::isGestureSupported(evt);
}

void Game::registerGesture(Gesture::GestureEvent evt)
{
    Platform::registerGesture(evt);
}

void Game::unregisterGesture(Gesture::GestureEvent evt)
{
    Platform::unregisterGesture(evt);
}

bool Game::isGestureRegistered(Gesture::GestureEvent evt)
{
    return Platform::isGestureRegistered(evt);
}

void Game::gestureSwipeEvent(int x, int y, int direction)
{
}

void Game::gesturePinchEvent(int x, int y, float scale)
{
}

void Game::gestureTapEvent(int x, int y)
{
}

#ifdef ENABLE_GAMEPAD
void Game::gamepadEvent(Gamepad::GamepadEvent evt, Gamepad* gamepad)
{
}
#endif

void Game::getArguments(int* argc, char*** argv) const
{
    Platform::getArguments(argc, argv);
}

void Game::schedule(float timeOffset, TimeListener* timeListener, void* cookie)
{
    GP_ASSERT(_timeEvents);
    TimeEvent timeEvent(getGameTime() + timeOffset, timeListener, cookie);
    _timeEvents->push(timeEvent);
}

void Game::schedule(float timeOffset, const char* function)
{
#ifdef ENABLE_LUA
    if (!_scriptListeners)
        _scriptListeners = new std::vector<ScriptListener*>();

    ScriptListener* listener = new ScriptListener(function);
    _scriptListeners->push_back(listener);
    schedule(timeOffset, listener, NULL);
#endif
}

void Game::fireTimeEvents(double frameTime)
{
    while (_timeEvents->size() > 0)
    {
        const TimeEvent* timeEvent = &_timeEvents->top();
        if (timeEvent->time > frameTime)
        {
            break;
        }
        if (timeEvent->listener)
        {
            timeEvent->listener->timeEvent(frameTime - timeEvent->time, timeEvent->cookie);
        }
        _timeEvents->pop();
    }
}

#ifdef ENABLE_LUA
Game::ScriptListener::ScriptListener(const char* url)
{
    function = Game::getInstance()->getScriptController()->loadUrl(url);
}

void Game::ScriptListener::timeEvent(long timeDiff, void* cookie)
{
    Game::getInstance()->getScriptController()->executeFunction<void>(function.c_str(), "l", timeDiff);
}
#endif

Game::TimeEvent::TimeEvent(double time, TimeListener* timeListener, void* cookie)
    : time(time), listener(timeListener), cookie(cookie)
{
}

bool Game::TimeEvent::operator<(const TimeEvent& v) const
{
    // The first element of std::priority_queue is the greatest.
    return time > v.time;
}

Properties* Game::getConfig() const
{
    if (_properties == NULL)
        const_cast<Game*>(this)->loadConfig();

    return _properties;
}

void Game::loadConfig()
{
    if (_properties == NULL)
    {
        // Try to load custom config from file.
        if (FileSystem::fileExists("game.config"))
        {
            _properties = Properties::create("game.config");

            // Load filesystem aliases.
            Properties* aliases = _properties->getNamespace("aliases", true);
            if (aliases)
            {
                FileSystem::loadResourceAliases(aliases);
            }
        }
        else
        {
            // Create an empty config
            _properties = new Properties();
        }
    }
}

void Game::loadGamepads()
{
    // Load virtual gamepads.
    if (_properties)
    {
        // Check if there are any virtual gamepads included in the .config file.
        // If there are, create and initialize them.
        _properties->rewind();
        Properties* inner = _properties->getNextNamespace();
        while (inner != NULL)
        {
            std::string spaceName(inner->getNamespace());
#ifdef ENABLE_GAMEPAD
			// This namespace was accidentally named "gamepads" originally but we'll keep this check
            // for backwards compatibility.
            if (spaceName == "gamepads" || spaceName == "gamepad")
            {
                if (inner->exists("form"))
                {
                    const char* gamepadFormPath = inner->getString("form");
                    GP_ASSERT(gamepadFormPath);
                    Gamepad* gamepad = Gamepad::add(gamepadFormPath);
                    GP_ASSERT(gamepad);
                }
            }
#endif
            inner = _properties->getNextNamespace();
        }
    }
}

void Game::ShutdownListener::timeEvent(long timeDiff, void* cookie)
{
	Game::getInstance()->shutdown();
}

}

