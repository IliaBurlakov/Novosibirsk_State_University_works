#include "Bullet.h"
#include "enemy.h"
#include "Box.h"
#define ENEMY_DAMAGE 5
#define HERO_DAMAGE 10
using namespace ShooterGame;

Bullet::Bullet(System::Drawing::Point startPosition, bool isPlayerProj, System::Collections::Generic::List<GameObject^>^ gameObjs)
{
    position = startPosition;
    width = 10;
    height = 5;
    fromHero = isPlayerProj;
    gameObjects = gameObjs;
    if (fromHero)
    {
        dx = 15;   
    }
    else
    {
        dx = -15;   
    }
}

void Bullet::act()
{
    position.X += dx;  

    if (fromHero)
    {
        for (int i = 0; i < gameObjects->Count; i++) {
            GameObject^ obj = gameObjects[i];

            if (obj->isEnemy())
            {
                if (((position.X + width) > obj->GetPosition().X )&& (position.X < (obj->GetPosition().X + 100)))
                {
                    if (((position.Y + height) > obj->GetPosition().Y) && (position.Y < (obj->GetPosition().Y + 100))) {
                        Enemy^ enemy = dynamic_cast<Enemy^>(obj);
                        if (enemy != nullptr) {
                            enemy->takeDamage(HERO_DAMAGE);  
                        }
                        gameObjects->Remove(this);  
                    }
                }
            }
            else if (obj->isBox())
            {
                if (((position.X + width) > obj->GetPosition().X) && (position.X < (obj->GetPosition().X + 100)))
                {
                    if (((position.Y + height) > obj->GetPosition().Y) && (position.Y < (obj->GetPosition().Y + 100))) {
                        Box^ box = dynamic_cast<Box^>(obj);
                        if (box != nullptr) {
                            box->takeDamage(HERO_DAMAGE);  
                        }
                        gameObjects->Remove(this);  
                    }
                }
            }
        }
    }
    else
    {
        for (int i = 0; i < gameObjects->Count; i++) {
            GameObject^ obj = gameObjects[i];

            if (obj->isHero())
            {
                if (((position.X + width) > obj->GetPosition().X) && (position.X < (obj->GetPosition().X + 100)))
                {
                    if (((position.Y + height) > obj->GetPosition().Y) && (position.Y < (obj->GetPosition().Y + 100))) {
                        Hero^ hero = dynamic_cast<Hero^>(obj); 
                        if (hero != nullptr) {
                            hero->takeDamage(ENEMY_DAMAGE); 
                        }
                        gameObjects->Remove(this);  
                    }
                }
            }
        }
    }


    if (position.X < 0 || position.X > 1400)
    {
        gameObjects->Remove(this);  
    }
}


void Bullet::draw(System::Drawing::Graphics^ g)
{
    g->FillRectangle(System::Drawing::Brushes::Orange, position.X, position.Y, width, height);
}
