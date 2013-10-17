/*
 * glscene.h
 *
 *  Created on: Aug 6, 2013
 *      Author: vlopatin
 */

#ifndef GLSCENE_H_
#define GLSCENE_H_

#include <lvptrvec.h>

class GLSceneItem {
public:
	// override to draw item
	virtual void draw() = 0;
	virtual ~GLSceneItem() { }
};

class GLScene {
	LVPtrVector<GLSceneItem, true> _list;
public:
	/// adds item to scene
	virtual void add(GLSceneItem * item);
	/// draws all items of scene
	virtual void draw();
	/// removes all items from scene
	virtual void clear();

	virtual ~GLScene();
};

void LVGLAddSceneItem(GLSceneItem * item);

typedef void (*last_scene_callback_t)();

/// sets callback to call on last scene end of drawing
void LVGLSetLastSceneCallback(last_scene_callback_t callback);

/// pushes new scene to scene stack, makes it current, returns passed value
GLScene * LVGLPushScene(GLScene * scene);
/// pops last scene from scene stack, makes previous scene current, returns popped scene
GLScene * LVGLPopScene();
/// returns top scene from scene stack, returns NULL of no current scene
GLScene * LVGLPeekScene();


#endif /* GLSCENE_H_ */
