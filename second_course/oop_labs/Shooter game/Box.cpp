#include "Box.h"
#include "Hero.h"
#include "GameForm.h"

namespace ShooterGame {

    Box::Box(System::Drawing::Point initialPosition, Hero^ heroObject, System::Collections::Generic::List<GameObject^>^ gameObj)
    {
        position = initialPosition;
        width = height = 100;
        hero = heroObject;
        health = 100;
        gameObjects = gameObj;
        boxImage = gcnew System::Drawing::Bitmap("Resources\\targetBox.png");
    }

    void Box::act()
    {
        position.X -= 3;
        if (position.X < 0)
            gameObjects->Remove(this);

        if (position.Y <= (hero->getHeroPosition().Y + 100) && position.Y >= (hero->getHeroPosition().Y - 100))
        {
            if (position.X <= (hero->getHeroPosition().X + 100) && position.X >= (hero->getHeroPosition().X - 100)) {
                position.X = -100;
                GameOverEvent();
            }
        }
    }

    int Box::getHealth() {
        return this->health;
    }

    void Box::takeDamage(int damage)
    {
        health -= damage;
        if (health <= 0)
        {
            gameObjects->Remove(this);  
            BoxDestroyedEvent(); 
        }
    }

    void Box::draw(System::Drawing::Graphics^ g)
    {
        g->DrawImage(boxImage, position); 
    }

}
