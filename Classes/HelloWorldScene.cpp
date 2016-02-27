#include "HelloWorldScene.h"
#include "GameOverScene.h"
#include "SimpleAudioEngine.h" 

USING_NS_CC;

Scene* HelloWorld::createScene()
{
	auto scene = Scene::createWithPhysics();
	//scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	Vect gravity(0.0f, 0.0f);
	scene->getPhysicsWorld()->setGravity(gravity);
	auto layer = HelloWorld::create();
	layer->setPhyWorld(scene->getPhysicsWorld());

	scene->addChild(layer);
	return scene;
}

bool HelloWorld::init()
{
	if (!Layer::init())
	{
		return false;
	}

	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto origin = Director::getInstance()->getVisibleOrigin();

	auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
	audio->preloadEffect("hit_paddle.wav");

	edgeSp = Sprite::create();

	//Set Physics
	auto boundBody = PhysicsBody::createEdgeBox(visibleSize, PHYSICSBODY_MATERIAL_DEFAULT, 3);// Create physics body
	boundBody->getShape(0)->setRestitution(1.0f);
	boundBody->getShape(0)->setFriction(0.0f);
	boundBody->getShape(0)->setDensity(1.0f);
	edgeSp->setPosition(Point(visibleSize.width / 2, visibleSize.height / 2)); // Set the position and the centre of Box in the center of the screen
	edgeSp->setPhysicsBody(boundBody); // Set physics Body
	boundBody->setContactTestBitmask(0x000001); // This is the important command, if not available, there is nothing happening when colliding
	this->addChild(edgeSp); // Add into Layer
	edgeSp->setTag(0); // Tag==0, to check object when colliding belongs to some kind

	//Create Ball
	ball = Sprite::create("ball.png", Rect(0, 0, 20, 20));
	ball->setPosition(100, 100);
	auto ballBody = PhysicsBody::createCircle(ball->getContentSize().width / 2); // The physics body circle shape
	ballBody->getShape(0)->setRestitution(1.0f);
	ballBody->getShape(0)->setFriction(0.0f);
	ballBody->getShape(0)->setDensity(8.0f);
	ballBody->setGravityEnable(false); // Not set acceleration
	Vect force = Vect(1010000.0f, 1010000.0f); // Create a force Vector to act with the direction of 45 degree, because x = y 
	ballBody->applyImpulse(force); // Push a force into the ball edge
	ball->setPhysicsBody(ballBody); // Set Physics body
	ballBody->setContactTestBitmask(0x000001); //
	ball->setTag(1);
	this->addChild(ball);

	//Create Paddle
	paddle = Sprite::create("paddle.png");
	auto paddleBody = PhysicsBody::createBox(paddle->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
	paddleBody->getShape(0)->setRestitution(1.0f);
	paddleBody->getShape(0)->setFriction(0.0f);
	paddleBody->getShape(0)->setDensity(10.0f);
	paddleBody->setGravityEnable(false);
	paddleBody->setDynamic(false); // Set static when reacting, no restitution, no changing position
	paddle->setPosition(visibleSize.width / 2, 50);
	paddle->setPhysicsBody(paddleBody);
	paddleBody->setContactTestBitmask(0x000001);
	audio->playEffect("hit_paddle.wav"); // With reaction 
	ball->setTag(2);
	this->addChild(paddle);

	//Create Bricks
	for (int i = 0; i < 19; i++) 
	{
		for (int j = 0; j < 5; j++)
		{
			static int xPadding = 50;
			static int yPadding = 50;
			auto block = Sprite::create();
			switch (cocos2d::RandomHelper::random_int(1, 5))
			{
				case 1: 
					block = Sprite::create("brck_b.png");
					break;
				case 2:
					block = Sprite::create("brck_r.png");
					break;
				case 3:
					block = Sprite::create("brck_g.png");
					break;
				case 4:
					block = Sprite::create("brck_y.png");
					break;
				case 5:
					block = Sprite::create("brck_p.png");
					break;
			}
			auto blockBody = PhysicsBody::createBox(block->getContentSize(), PHYSICSBODY_MATERIAL_DEFAULT);
			blockBody->getShape(0)->setDensity(10.0f);
			blockBody->getShape(0)->setFriction(0.0f);
			blockBody->getShape(0)->setRestitution(1.f);
			blockBody->setDynamic(false);

			int xPos = xPadding + (block->getContentSize().width * i);
			int yPos = yPadding + (block->getContentSize().height * j) + 370;

			block->setPosition(xPos, yPos);
			blockBody->setContactTestBitmask(0x000001);
			block->setPhysicsBody(blockBody);
			block->setTag(3);
			this->addChild(block);
		}
	}

	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->setSwallowTouches(true);
	touchListener->onTouchBegan = CC_CALLBACK_2(HelloWorld::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(HelloWorld::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(HelloWorld::onTouchEnded, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	auto dispatcher = Director::getInstance()->getEventDispatcher();
	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(HelloWorld::onContactBegin, this);
	dispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);


	return true;
}

bool HelloWorld::onTouchBegan(Touch* touch, Event* event)
{
	return true; // Not use but must return True
}

void HelloWorld::onTouchEnded(Touch* touch, Event* event)
{
	// Not use
}

// Use to move the paddle simplest
void HelloWorld::onTouchMoved(Touch* touch, Event* event) {
	Point touchLocation = this->convertToWorldSpace(this->convertTouchToNodeSpace(touch));
	// To be simple, use this command: Point touchLocation = touch->getLocation();
	paddle->setPositionX(touchLocation.x); // Set the position horizontal of the paddle follow the Touch point
}

bool HelloWorld::onContactBegin(PhysicsContact& contact)
{
	auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
	audio->preloadEffect("lose.wav");
	audio->preloadEffect("hit_brick.wav");

	// Get two collided object
	auto spriteA = (Sprite*)contact.getShapeA()->getBody()->getNode();
	auto spriteB = (Sprite*)contact.getShapeB()->getBody()->getNode();

	// Check kinds of objects
	int tagA = spriteA->getTag();
	int tagB = spriteB->getTag();

	if (tagA == 3) // is brick
	{

		this->removeChild(spriteA, true); 
		audio->playEffect("hit_brick.wav"); // delete brick

										  //spriteA->removeFromParentAndCleanup(true);
	}

	if (tagB == 3)  // is brick
	{
		this->removeChild(spriteB, true);
		audio->playEffect("hit_brick.wav"); // delete brick

										  //spriteB->removeFromParentAndCleanup(true);
	}

	// If the ball collides with the floor and the coordinate Y of the ball is smaller than the paddle, Game Over happens
	if ((tagA == 0 || tagB == 0)& (ball->getPositionY() <= paddle->getPositionY()))
	{
		auto gameOverScene = GameOverScene::create();
		audio->playEffect("lose.wav");
		gameOverScene->getLayer()->getLabel()->setString("GAME OVER");
		Director::getInstance()->replaceScene(gameOverScene);
	}

	return true;
}