#pragma once

#include "cocos2d.h"

USING_NS_CC;

class HelloWorld : public cocos2d::Layer
{
public:
	Sprite* ball;
	Sprite* paddle;
	Sprite* edgeSp;

	PhysicsWorld* m_world;

	void setPhyWorld(PhysicsWorld* world) { m_world = world; };
	// Touch event
	void onTouchMoved(Touch *touch, Event *event);
	void onTouchEnded(Touch *touch, Event *event);

	bool onTouchBegan(Touch *touch, Event *event);

	static cocos2d::Scene* createScene();
	virtual bool init();

	CREATE_FUNC(HelloWorld);

};