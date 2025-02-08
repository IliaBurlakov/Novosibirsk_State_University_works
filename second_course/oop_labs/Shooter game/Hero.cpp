#include "Hero.h"
using namespace ShooterGame;

Hero::Hero(System::Drawing::Point initialPosition, System::Collections::Generic::List<GameObject^>^ gameObjs)
{
	position = initialPosition;
	width = height = 100;
	heroImage = gcnew System::Drawing::Bitmap("Resources\\hero.png");
	movingUp = false;
	movingDown = false;
	dy = 0;
	health = 100;
	gameObjects = gameObjs;
}
System::Drawing::Point Hero::getHeroPosition() {
	return this->position;
}
int Hero::getHeroHP() {
	return this->health;
}
void Hero::act()
{
	if (movingUp)
	{
		dy = -15;
	}
	else if (movingDown)
	{
		dy = 15; 
	}
	else
	{
		dy = 0;
	}

	position.Y += dy;

	if (position.Y < 0) position.Y = 0;
	if (position.Y > 690) position.Y = 690;
}

void Hero::takeDamage(int damage)
{
	health -= damage; 
	if (health <= 0) {
		gameObjects->Remove(this);
		GameOverEvent();
	}
}
void Hero::heal(int hillPoints) {
	if ((health + hillPoints) >= 100)
		health = 100;
	else
		health += hillPoints;
}
void Hero::draw(System::Drawing::Graphics^ g)
{
	g->DrawImage(heroImage, position);
}

void Hero::startMoveUp()
{
	movingUp = true;  
}

void Hero::stopMoveUp()
{
	movingUp = false;
}

void Hero::startMoveDown()
{
	movingDown = true;  
}

void Hero::stopMoveDown()
{
	movingDown = false;
}

void Hero::shoot()
{
	Bullet^ bullet = gcnew Bullet(System::Drawing::Point(position.X + width, position.Y + height / 2), true, gameObjects);
	gameObjects->Add(bullet);
}