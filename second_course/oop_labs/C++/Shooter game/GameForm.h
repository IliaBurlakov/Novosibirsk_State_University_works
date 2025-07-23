#pragma once
#include <time.h>
#include <stdlib.h>
#include <vector>
#include "Hero.h"
#include "Enemy.h"
#include "Box.h"

namespace ShooterGame {

    public ref class GameForm : public System::Windows::Forms::Form
    {
    public:
        GameForm();
        virtual ~GameForm();
        void SetPlayerNickname(System::String^ nickname) { playerNickname = nickname; }
        void SetHighScores(System::Collections::Generic::Dictionary<System::String^, int>^ scores) { highScores = scores; }
    protected:
        virtual void OnPaint(System::Object^ obj, System::Windows::Forms::PaintEventArgs^ e) override;
        virtual void OnKeyDown(System::Object^ obj, System::Windows::Forms::KeyEventArgs^ e) override;
        virtual void OnKeyUp(System::Object^ obj, System::Windows::Forms::KeyEventArgs^ e) override;

    private:
        int score;
        int boxesDestroyed;
        bool gamePaused;  
        Hero^ hero;
        

        int CompareHighScores(System::Collections::Generic::KeyValuePair<System::String^, int> a, System::Collections::Generic::KeyValuePair<System::String^, int> b);


        System::Windows::Forms::Timer^ timer;  // Таймер игрового цикла
        System::Windows::Forms::Timer^ enemyGenTimer;  // Таймер для генерации врагов
        System::ComponentModel::Container^ components;
        System::Collections::Generic::List<GameObject^>^ gameObjects;  // Список для хранения объектов игры
        System::Windows::Forms::Label^ pauseLabel;

        void InitializeComponent(void);
        void GameLoop(System::Object^ sender, System::EventArgs^ e);  // Игровой цикл
        void CreateGameLoopTimer();
        void CreateEnemyGenTimer();
        void EnterFullScreen();
        void ExitFullScreen();
        void TogglePause();
        void GenerateEnemyOrBox(System::Object^ sender, System::EventArgs^ e);
        bool CheckCollision(System::Drawing::Point newPosition, int width, int height);
        bool IsCollision(System::Drawing::Point newPosition, int width, int height, System::Drawing::Point objPosition);  // Проверка на столкновение
        System::Drawing::Point GenerateRandomPosition();

        void GameOver();
        void GameWin();
        void RestartGame();
        void OnEnemyDestroyed();
        void OnBoxDestroyed();

        System::Collections::Generic::Dictionary<System::Windows::Forms::Keys, System::Action^>^ keyDownActions;
        System::Collections::Generic::Dictionary<System::Windows::Forms::Keys, System::Action^>^ keyUpActions;
        System::String^ playerNickname;
        System::Collections::Generic::Dictionary<System::String^, int>^ highScores;
    };
}
