#pragma once
#include <Windows.h>

namespace ShooterGame {

    public ref class GameObject
    {
    public:
        virtual ~GameObject() {}
        virtual void act() = 0;
        virtual void draw(System::Drawing::Graphics^ g) = 0;

        System::Drawing::Point GetPosition() { return position; }
        void SetPosition(System::Drawing::Point pos) { position = pos; }

        virtual bool isEnemy() { return false; }
        virtual bool isHero() { return false; }
        virtual bool isBox() { return false; }

    protected:
        System::Drawing::Point position; 
        int width, height;
    };
}
