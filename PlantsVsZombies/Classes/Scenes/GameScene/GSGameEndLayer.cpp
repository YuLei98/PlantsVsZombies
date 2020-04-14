/**
 *Copyright (c) 2020 LZ.All Right Reserved
 *Author : LZ
 *Date: 2020.2.14
 *Email: 2117610943@qq.com
 */

#include "GSGameEndLayer.h"
#include "GSData.h"
#include "GSInformationLayer.h"
#include "GSPauseQuitLayer.h"

#include "Scenes/WorldScene/World_1.h"
#include "Based/GlobalVariable.h"
#include "Based/UserWinRequirement.h"
#include "Based/GameType.h"
#include "Based/LevelData.h"
#include "Based/Car.h"

GSGameEndLayer::GSGameEndLayer():
	_userWinRequirement(nullptr)
,	_global(Global::getInstance())
,   _openLevelData(OpenLevelData::getInstance())
{
	srand(time(nullptr));
}

GSGameEndLayer::~GSGameEndLayer()
{
	if (_userWinRequirement)delete _userWinRequirement;
}

bool GSGameEndLayer::init()
{
    if (!LayerColor::initWithColor(Color4B(0,0,0,180)))return false;

    createShieldLayer();

    return true;
}

void GSGameEndLayer::createShieldLayer()
{
    auto shieldListener = EventListenerTouchOneByOne::create();
    shieldListener->onTouchBegan = [](Touch* touch, Event* event)-> bool { return true; };
    shieldListener->setSwallowTouches(true);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(shieldListener, this);
}

void GSGameEndLayer::successfullEntry()
{
	Director::getInstance()->getScheduler()->setTimeScale(1.0f); /* 恢复播放倍数 */
	caveLevelNumber();
	carsToCoins();

	auto moneybag_hi_res = ui::Button::create("moneybag_hi_res.png", "", "", TextureResType::PLIST);
	moneybag_hi_res->setPosition(Vec2(700 + rand() % 1000, 100 + rand() % 700));
	moneybag_hi_res->runAction(JumpBy::create(0.5f, Vec2(100 - rand() % 200, 0), 100 + rand() % 100, 1));
	moneybag_hi_res->setName("moneybag_hi_res");
	moneybag_hi_res->setLocalZOrder(2);
	this->addChild(moneybag_hi_res);

	moneybag_hi_res->runAction(RepeatForever::create(Sequence::create(TintTo::create(0.3f, 150, 150, 150), TintTo::create(0.3f, 255, 255, 255), nullptr)));
	moneybag_hi_res->addTouchEventListener([this, moneybag_hi_res](Ref* sender, ui::Widget::TouchEventType type)
		{
			switch (type)
			{
			case cocos2d::ui::Widget::TouchEventType::BEGAN:
				AudioEngine::setVolume(AudioEngine::play2d(_global->userInformation->getMusicPath().find("coin")->second), _global->userInformation->getSoundEffectVolume());
				break;
			case cocos2d::ui::Widget::TouchEventType::ENDED:
				moneybag_hi_res->setEnabled(false);
				moneybag_hi_res->setBright(true);

				AudioEngine::setVolume(AudioEngine::play2d(_global->userInformation->getMusicPath().find("moneyfalls")->second), _global->userInformation->getSoundEffectVolume());
				rewardCoin(moneybag_hi_res);
				break;
			}
		});
}

void GSGameEndLayer::breakThrough(GameTypes gameType)
{
	GSPauseQuitLayer::pauseLayer();
	Director::getInstance()->getScheduler()->setTimeScale(1.0f); /* 恢复播放倍数 */
	UserDefault::getInstance()->setIntegerForKey("BREAKTHROUGH", ++_global->userInformation->getBreakThroughnumbers());/* 记录闯关失败个数 */

	AudioEngine::setVolume(AudioEngine::play2d(_global->userInformation->getMusicPath().find("losemusic")->second), _global->userInformation->getSoundEffectVolume());

	if (gameType == GameTypes::UserLose)
	{
		showFailText();
	}
	else
	{
		showFailDialog(gameType);
	}
}

void GSGameEndLayer::showFailDialog(GameTypes gameType)
{
	_userWinRequirement = new UserWinRequirement(this);
	_userWinRequirement->createDialogBox(gameType);
	_userWinRequirement->setShowDialogAction();

	auto button = Button::create("Continue1.png", "Continue.png", "", TextureResType::PLIST);
	button->setTitleText(Global::getInstance()->userInformation->getGameText().find("继续")->second);
	button->setTitleFontName(GAME_FONT_NAME_1);
	button->setTitleFontSize(30);
	button->setTitleColor(Color3B::YELLOW);
	button->setPosition(Vec2(_userWinRequirement->getDialog()->getContentSize().width / 2.0f, 10));
	button->setGlobalZOrder(10);
	button->setScale(0.5f);
	button->runAction(FadeIn::create(0.5f));
	button->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type)
		{
			switch (type)
			{
			case ui::Widget::TouchEventType::BEGAN:
				AudioEngine::setVolume(AudioEngine::play2d(Global::getInstance()->userInformation->getMusicPath().find("gravebutton")->second), Global::getInstance()->userInformation->getSoundEffectVolume());
				break;
			case ui::Widget::TouchEventType::ENDED:
				_userWinRequirement->setDelectDialogAction();
				_director->replaceScene(TransitionFade::create(1.0f, World_1::createScene()));
				break;
			}
		});
	_userWinRequirement->getDialog()->addChild(button);
}

void GSGameEndLayer::showFailText()
{
	auto brain = Sprite::createWithSpriteFrameName("Brain.png");
	brain->setPosition(_director->getWinSize() / 2.0f);
	brain->setScale(2.0f);
	brain->runAction(RepeatForever::create(Sequence::create(MoveBy::create(0.05f, Vec2(10, 0)), MoveBy::create(0.05f, Vec2(-10, 0)), nullptr)));
	this->addChild(brain);

	auto lose = Sprite::createWithSpriteFrameName(rand() % 2 == 0 ? "zombieswon.png" : "zombieswon1.png");
	lose->setScale(2.0f);
	lose->setPosition(Vec2(2500, Director::getInstance()->getWinSize().height / 2.0f));
	lose->runAction(RepeatForever::create(Sequence::create(MoveBy::create(0.05f, Vec2(10, 0)), MoveBy::create(0.05f, Vec2(-10, 0)), nullptr)));
	lose->runAction(Sequence::create(JumpTo::create(1.0f, Director::getInstance()->getWinSize() / 2.0f, 250, 3),
		DelayTime::create(5),CallFunc::create([this]()
			{
				_director->replaceScene(TransitionFade::create(1.0f, World_1::createScene()));
			}),
		nullptr));
	this->addChild(lose);
}

void GSGameEndLayer::caveLevelNumber()
{
	/* 记录关数 */
	if (_global->userInformation->getUserSelectWorldData().at(
		_global->userInformation->getCurrentPlayWorldTag())->levels == _global->userInformation->getCurrentPlayLevels())
	{
		UserDefault::getInstance()->setIntegerForKey(
			_global->userInformation->getGameDifficulty() ?
			_global->userInformation->getSystemDifCaveFileName(_global->userInformation->getUserCaveFileNumber()).c_str() :
			_global->userInformation->getSystemCaveFileName(_global->userInformation->getUserCaveFileNumber()).c_str(),
			++_global->userInformation->getUserSelectWorldData().at(0)->levels);
		
	}
}

void GSGameEndLayer::carsToCoins()
{
	int i = -1;
	for (auto car : CarsGroup)
	{
		if (!car->getLive())
		{
			auto cars = car;
			runAction(Sequence::create(DelayTime::create(0.4f * ++i), CallFunc::create([this, cars]()
				{
					AudioEngine::setVolume(AudioEngine::play2d(_global->userInformation->getMusicPath().find("coin")->second), _global->userInformation->getSoundEffectVolume());
					cars->getCar()->runAction(Sequence::create( Spawn::create(ScaleTo::create(0.2f, 0), FadeOut::create(0.2f), nullptr), nullptr));
					cars->getCar()->setVisible(false);
					coinAction(cars->getCar()->getPosition(), 0, true);
				}), nullptr));
		}
	}
}

void GSGameEndLayer::rewardCoin(Button* button)
{
	const int coins = _openLevelData->readLevelData(_openLevelData->getLevelNumber())->getCoinNumbers() / 2;
	const int number = coins + rand() % coins;

	button->runAction(Sequence::create(Repeat::create(Sequence::create(MoveBy::create(0.05f, Vec2(5, 5)), 
		MoveBy::create(0.05f, Vec2(-5, -5)), nullptr), number / 2), DelayTime::create(0.5f), FadeOut::create(0.5f),
		CallFunc::create([button]()
			{
				button->removeFromParent();
			}),
		nullptr));

	for (int i = 0; i < number; i++)
	{
		coinAction(button->getPosition() - Vec2(-10, 55), i);

		if (i >= number - 1)
		{
			this->runAction(Sequence::create(DelayTime::create(2.0f),
				CallFunc::create([=]()
					{
						auto audio = AudioEngine::play2d(_global->userInformation->getMusicPath().find("winmusic")->second);
						AudioEngine::setVolume(audio, _global->userInformation->getSoundEffectVolume());
						AudioEngine::setFinishCallback(audio, [=](int i, string name)
							{
								AudioEngine::setVolume(AudioEngine::play2d(_global->userInformation->getMusicPath().find("lightfill")->second), _global->userInformation->getSoundEffectVolume());
							});

						auto AwardRays = Sprite::createWithSpriteFrameName("AwardRays.png");
						AwardRays->setPosition(_director->getWinSize() / 2.0f);
						AwardRays->runAction(RepeatForever::create(Spawn::create(RotateBy::create(0.5f, 30), ScaleBy::create(0.5f, 1.5f), nullptr)));
						AwardRays->setGlobalZOrder(20);
						this->addChild(AwardRays);

						auto White = Sprite::createWithSpriteFrameName("White.png");
						White->setAnchorPoint(Vec2(0, 0));
						White->setContentSize(_director->getWinSize());
						White->setOpacity(0);
						White->setGlobalZOrder(20);
						this->addChild(White);
						White->runAction(Sequence::create(DelayTime::create(1.0f), FadeIn::create(7.0f),
							CallFunc::create([=]()
								{
									/* 保存金币数 */
									UserDefault::getInstance()->setIntegerForKey("COINNUMBERS", _global->userInformation->getCoinNumbers() << 10);
									
									_director->replaceScene(TransitionFade::create(1.0f, World_1::createScene()));
								}), nullptr));
					}), nullptr));
		}
	}
}

void GSGameEndLayer::coinAction(const Vec2& position, const int id, const bool big)
{
	auto coin = SkeletonAnimation::createWithData(_global->userInformation->getAnimationData().find("coin")->second);
	coin->setPosition(position);
	coin->setScale(0.05f);
	this->addChild(coin);

	auto callFunc = CallFunc::create([=]()
		{
			AudioEngine::setVolume(AudioEngine::play2d(
				_global->userInformation->getMusicPath().find("moneyfalls")->second),
				_global->userInformation->getSoundEffectVolume());
			coin->removeFromParent();
			big ? _global->userInformation->setCoinNumbers(_global->userInformation->getCoinNumbers() + 10) :
				_global->userInformation->setCoinNumbers(_global->userInformation->getCoinNumbers() + 1);
			informationLayerInformation->updateCoinNumbers();
		});
	
	auto action = Spawn::create(FadeIn::create(0.5f), ScaleTo::create(0.5f, 0.3f), JumpBy::create(0.5f, Vec2(150 - rand() % 300, 100 - rand() % 100), rand() % 100 + 200, 1),
		Sequence::create(DelayTime::create(0.25f), CallFunc::create([coin]() {coin->setLocalZOrder(3); }), nullptr), nullptr);
	auto action1 = Spawn::create(ScaleTo::create(0.2f, 0.5f), FadeOut::create(0.2f), callFunc, nullptr);

	coin->runAction(Sequence::create(DelayTime::create(id * 0.05f), action, DelayTime::create(0.05f), MoveTo::create(0.5f, Vec2(1650, -40)), action1, nullptr));
}
