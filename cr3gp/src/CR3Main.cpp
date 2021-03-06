#include "CR3Main.h"

// Declare our game instance
CR3Main game;

CR3Main::CR3Main()
    : _scene(NULL)
{
	//getConfig();
}

void CR3Main::initialize()
{
    // Load game scene from file
    _scene = Scene::load("res/box.gpb");
	_form = Form::create("res/ui/forms/formVerticalLayout.form"); 

    // Set the aspect ratio for the scene's camera to match the current resolution
    _scene->getActiveCamera()->setAspectRatio(getAspectRatio());
    
    // Get light node
    Node* lightNode = _scene->findNode("directionalLight");
    Light* light = lightNode->getLight();

    // Initialize box model
    Node* boxNode = _scene->findNode("box");
    Model* boxModel = boxNode->getModel();
    Material* boxMaterial = boxModel->setMaterial("res/box.material");
    boxMaterial->getParameter("u_ambientColor")->setValue(_scene->getAmbientColor());
    boxMaterial->getParameter("u_lightColor")->setValue(light->getColor());
    boxMaterial->getParameter("u_lightDirection")->setValue(lightNode->getForwardVectorView());
}

void CR3Main::finalize()
{
    SAFE_RELEASE(_scene);
    SAFE_RELEASE(_form);
}

void CR3Main::update(float elapsedTime)
{
    // Rotate model
    _scene->findNode("box")->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
	_form->update(elapsedTime);
}

void CR3Main::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);

    // Visit all the nodes in the scene for drawing
    _scene->visit(this, &CR3Main::drawScene);
	_form->draw();
}

bool CR3Main::drawScene(Node* node)
{
    // If the node visited contains a model, draw it
    Model* model = node->getModel(); 
    if (model)
    {
        model->draw();
    }
    return true;
}

void CR3Main::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if (evt == Keyboard::KEY_PRESS)
    {
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
		case Keyboard::KEY_RETURN:
			//Platform::
            break;
        }
    }
}

void CR3Main::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}

void CR3Main::resizeEvent(unsigned int width, unsigned int height)
{
	Game::resizeEvent(width, height);
}
