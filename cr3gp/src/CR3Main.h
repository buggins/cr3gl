#ifndef CR3Main_H_
#define CR3Main_H_

#include "gameplay.h"

using namespace gameplay;

/**
 * Main game class.
 */
class CR3Main: public Game
{
public:

    /**
     * Constructor.
     */
    CR3Main();

    /**
     * @see Game::keyEvent
     */
	void keyEvent(Keyboard::KeyEvent evt, int key);
	
    /**
     * @see Game::touchEvent
     */
    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

protected:

    /**
     * @see Game::initialize
     */
    void initialize();

    /**
     * @see Game::finalize
     */
    void finalize();

    /**
     * @see Game::update
     */
    void update(float elapsedTime);

    /**
     * @see Game::render
     */
    void render(float elapsedTime);

	void resizeEvent(unsigned int width, unsigned int height);

private:

    /**
     * Draws the scene each frame.
     */
    bool drawScene(Node* node);

    Scene* _scene;
	Form* _form;
};

#endif
