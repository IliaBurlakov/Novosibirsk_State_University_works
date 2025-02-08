#pragma once
#include "GameObject.h"
#include "Hero.h"
#include "Bullet.h"

namespace ShooterGame {

    public ref class Box : public GameObject
    {
    public:
        Box(System::Drawing::Point initialPosition, Hero^ heroObject, System::Collections::Generic::List<GameObject^>^ gameObj);
        virtual void act() override;
        virtual bool isBox() override { return true; }
        virtual void draw(System::Drawing::Graphics^ g) override;
        delegate void BoxDestroyedEventHandler();
        event BoxDestroyedEventHandler^ BoxDestroyedEvent;
        delegate void GameOverEventHandler();
        event GameOverEventHandler^ GameOverEvent;
        void takeDamage(int damage);
        int getHealth();
    private:
        int health;
        Hero^ hero;
        System::Collections::Generic::List<GameObject^>^ gameObjects;
        System::Drawing::Bitmap^ boxImage;
    };
}
