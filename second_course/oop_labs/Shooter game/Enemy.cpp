#include "Enemy.h"
#include "GameForm.h"

namespace ShooterGame {

    Enemy::Enemy(System::Drawing::Point initialPosition, Hero^ heroObject, System::Collections::Generic::List<GameObject^>^ gameObjs)
    {
        position = initialPosition;
        width = height = 100;
        health = 40;
        enemyImage = gcnew System::Drawing::Bitmap("Resources\\enemy.png");
        hero = heroObject;  
        gameObjects = gameObjs; 

        lastShotTime = System::DateTime::Now;
        shotCooldown = System::TimeSpan::FromMilliseconds(300);  // Интервал стрельбы 0.3 сек
    }

    void Enemy::takeDamage(int damage)
    {
        health -= damage; 
        if (health <= 0) {
            EnemyDestroyedEvent();
            gameObjects->Remove(this); 
        }
    }

    void Enemy::act()
    {
        position.X -= 3;  
        if (position.X < 0)
            gameObjects->Remove(this);

        // Проверка, можно ли стрелять
        if ((System::DateTime::Now - lastShotTime) >= shotCooldown)
        {
            // Стрельба, если враг на одном уровне с героем по оси Y
            if (position.Y <= (hero->getHeroPosition().Y + 100) && position.Y >= (hero->getHeroPosition().Y - 100)) // Проверка на одном уровне
            {
                if (position.X <= (hero->getHeroPosition().X + 100) && position.X >= (hero->getHeroPosition().X - 100)) {
                    position.X = -100;
                    GameOverEvent();
                }
                shoot(); 
                lastShotTime = System::DateTime::Now;
            }
        }
    }

    void Enemy::draw(System::Drawing::Graphics^ g)
    {
        g->DrawImage(enemyImage, position);
    }

    void Enemy::shoot()
    {
        Bullet^ bullet = gcnew Bullet(System::Drawing::Point(position.X, position.Y + height / 2), false, gameObjects);
        gameObjects->Add(bullet);
    }

}
