#pragma once
#include "GameObject.h"
#include "Bullet.h"
namespace ShooterGame {
    public ref class Hero : public GameObject
    {
    public:
        Hero(System::Drawing::Point initialPosition, System::Collections::Generic::List<GameObject^>^ gameObjs);
        virtual void act() override;
        virtual void draw(System::Drawing::Graphics^ g) override;
        void startMoveUp();
        void stopMoveUp();
        void startMoveDown();
        void stopMoveDown();
        void shoot();
        virtual bool isHero() override { return true; }
        delegate void GameOverEventHandler();
        event GameOverEventHandler^ GameOverEvent;
        System::Drawing::Point getHeroPosition();
        int getHeroHP();
        void takeDamage(int damage);
        void heal(int hillPoints);
    private:
        int healthPoints;
        System::Drawing::Bitmap^ heroImage;
        bool movingUp;
        bool movingDown;
        int dy;  // Скорость по оси Y
        int health;
        System::Collections::Generic::List <GameObject^>^ gameObjects;
    };
}
