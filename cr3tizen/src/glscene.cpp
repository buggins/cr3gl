/*
 * glscene.cpp
 *
 *  Created on: Aug 6, 2013
 *      Author: vlopatin
 */

#include "glscene.h"

static GLScene * currentGLScene = NULL;
static LVPtrVector<GLScene> sceneStack;

void GLScene::clear() {
	_list.clear();
}

/// adds item to scene
void GLScene::add(GLSceneItem * item) {
	_list.add(item);
}

void GLScene::draw() {
	for (int i=0; i<_list.length(); i++)
		_list[i]->draw();
}

GLScene::~GLScene() {
	_list.clear();
}

/// pushes new scene to scene stack, makes it current
GLScene * LVGLPushScene(GLScene * scene) {
	sceneStack.push(scene);
	currentGLScene = scene;
	return scene;
}

/// pops last scene from scene stack, makes previous scene current, returns popped scene
GLScene * LVGLPopScene() {
	GLScene * res = currentGLScene;
	currentGLScene = sceneStack.pop();
	return res;
}
/// returns top scene from scene stack, returns NULL of no current scene
GLScene * LVGLPeekScene() {
	return currentGLScene;
}

void LVGLAddSceneItem(GLSceneItem * item) {
	if (currentGLScene)
		currentGLScene->add(item);
}

