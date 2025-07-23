#pragma once
#include "GameObject.h"
#include "Hero.h"
#include "Bullet.h"

namespace ShooterGame {

    public ref class Enemy : public GameObject
    {
    public:
        Enemy(System::Drawing::Point initialPosition, Hero^ heroObject, System::Collections::Generic::List<GameObject^>^ gameObjs);
        delegate void GameOverEventHandler();
        event GameOverEventHandler^ GameOverEvent;
        delegate void EnemyDestroyedEventHandler();
        event EnemyDestroyedEventHandler^ EnemyDestroyedEvent;
        virtual void act() override;
        virtual void draw(System::Drawing::Graphics^ g) override;
        void shoot();
        virtual bool isEnemy() override { return true; }
        void takeDamage(int damage);
    private:
        int health;
        System::Drawing::Bitmap^ enemyImage;
        Hero^ hero;  
        System::Collections::Generic::List<GameObject^>^ gameObjects; 
        System::DateTime lastShotTime;
        System::TimeSpan shotCooldown;
    };
}
