#pragma once
#include "GameObject.h"

namespace ShooterGame {
    public ref class Bullet : public GameObject
    {
    public:
        Bullet(System::Drawing::Point startPosition, bool isPlayerProj, System::Collections::Generic::List<GameObject^>^ gameObjs);
        virtual void act() override;  
        virtual void draw(System::Drawing::Graphics^ g) override;

    private:
        System::Collections::Generic::List <GameObject^>^ gameObjects;
        bool fromHero; 
        int dx;  
    };

}
