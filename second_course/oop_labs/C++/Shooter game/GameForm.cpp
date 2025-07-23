#include "GameForm.h"
#define GAME_WIDTH 1400
#define GAME_HEIGHT 800
#define OBJECT_SIZE 100
#define TARGETS_TO_WIN 5
namespace ShooterGame {

    GameForm::GameForm()
    {
        this->WindowState = System::Windows::Forms::FormWindowState::Maximized;
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None;
        this->TopMost = true;
        this->KeyPreview = true;
        InitializeComponent();


        gamePaused = false;
        score = 0;
        boxesDestroyed = 0;
        gameObjects = gcnew System::Collections::Generic::List<GameObject^>();
        hero = gcnew Hero(System::Drawing::Point(0, 400), gameObjects);
        gameObjects->Add(hero);

        // Инициализация словарей
        keyDownActions = gcnew System::Collections::Generic::Dictionary<System::Windows::Forms::Keys, System::Action^>();
        keyUpActions = gcnew System::Collections::Generic::Dictionary<System::Windows::Forms::Keys, System::Action^>();

        // Действия при нажатии клавиш
        keyDownActions[System::Windows::Forms::Keys::Escape] = gcnew System::Action(this, &GameForm::ExitFullScreen);
        keyDownActions[System::Windows::Forms::Keys::F] = gcnew System::Action(this, &GameForm::EnterFullScreen);
        keyDownActions[System::Windows::Forms::Keys::P] = gcnew System::Action(this, &GameForm::TogglePause);
        keyDownActions[System::Windows::Forms::Keys::NumPad8] = gcnew System::Action(hero, &Hero::startMoveUp);
        keyDownActions[System::Windows::Forms::Keys::NumPad5] = gcnew System::Action(hero, &Hero::startMoveDown);
        keyDownActions[System::Windows::Forms::Keys::R] = gcnew System::Action(this, &GameForm::RestartGame);

        // Действия при отпускании клавиш
        keyUpActions[System::Windows::Forms::Keys::Space] = gcnew System::Action(hero, &Hero::shoot);
        keyUpActions[System::Windows::Forms::Keys::NumPad8] = gcnew System::Action(hero, &Hero::stopMoveUp);
        keyUpActions[System::Windows::Forms::Keys::NumPad5] = gcnew System::Action(hero, &Hero::stopMoveDown);

        hero->GameOverEvent += gcnew Hero::GameOverEventHandler(this, &GameForm::GameOver);
        Enemy^ enemy = gcnew Enemy(GenerateRandomPosition(), hero, gameObjects);
        enemy->GameOverEvent += gcnew Enemy::GameOverEventHandler(this, &GameForm::GameOver);
        enemy->EnemyDestroyedEvent += gcnew Enemy::EnemyDestroyedEventHandler(this, &GameForm::OnEnemyDestroyed);
        gameObjects->Add(enemy);
        CreateEnemyGenTimer();
        CreateGameLoopTimer();
    }

    GameForm::~GameForm()
    {
        if (components) {
            delete components;
        }
    }

    void GameForm::CreateEnemyGenTimer()
    {
        enemyGenTimer = gcnew System::Windows::Forms::Timer();
        enemyGenTimer->Interval = 1500;
        enemyGenTimer->Tick += gcnew System::EventHandler(this, &GameForm::GenerateEnemyOrBox);
        enemyGenTimer->Start();
    }

    void GameForm::CreateGameLoopTimer()
    {
        timer = gcnew System::Windows::Forms::Timer();
        timer->Interval = 10;
        timer->Tick += gcnew System::EventHandler(this, &GameForm::GameLoop);
        timer->Start();
    }

    void GameForm::GameOver()
    {
        timer->Stop();

        // Обновление рекорда игрока
        if (!highScores->ContainsKey(playerNickname) || score > highScores[playerNickname])
        {
            highScores[playerNickname] = score;
        }

        this->BackColor = System::Drawing::Color::DarkRed;
        System::Windows::Forms::MessageBox::Show("Game Over! Your score: " + score);
        RestartGame();
    }


    void GameForm::GameWin()
    {
        timer->Stop();

        // Обновление рекорда игрока
        if (!highScores->ContainsKey(playerNickname) || score > highScores[playerNickname])
        {
            highScores[playerNickname] = score;
        }

        this->BackColor = System::Drawing::Color::DarkGreen;
        System::Windows::Forms::MessageBox::Show("Congratulations! You Win! Your score: " + score);
        RestartGame();
    }


    System::Drawing::Point GameForm::GenerateRandomPosition()
    {
        static bool seeded = false;
        if (!seeded) {
            srand(time(NULL));
            seeded = true;
        }

        int randomX = 1400;
        int randomY = rand() % 691;
        return System::Drawing::Point(randomX, randomY);
    }


    bool GameForm::CheckCollision(System::Drawing::Point newPosition, int width, int height)
    {
        for (int i = 0; i < gameObjects->Count; i++) {
            GameObject^ obj = gameObjects[i];

            System::Drawing::Point objPosition = obj->GetPosition();

            if (IsCollision(newPosition, OBJECT_SIZE, OBJECT_SIZE, objPosition)) {
                return true;
            }
        }
        return false;
    }


    bool GameForm::IsCollision(System::Drawing::Point newPosition, int width, int height, System::Drawing::Point objPosition)
    {
        return (newPosition.X < (objPosition.X + OBJECT_SIZE)) &&
            ((newPosition.X + width) > objPosition.X) &&
            (newPosition.Y < (objPosition.Y + OBJECT_SIZE)) &&
            ((newPosition.Y + height) > objPosition.Y);
    }


    void GameForm::RestartGame()
    {
        score = 0;
        boxesDestroyed = 0;
        gameObjects->Clear();

        hero = gcnew Hero(System::Drawing::Point(0, 400), gameObjects);
        gameObjects->Add(hero);
        hero->GameOverEvent += gcnew Hero::GameOverEventHandler(this, &GameForm::GameOver);

        // Обновляем действия для нового героя
        keyDownActions[System::Windows::Forms::Keys::NumPad8] = gcnew System::Action(hero, &Hero::startMoveUp);
        keyDownActions[System::Windows::Forms::Keys::NumPad5] = gcnew System::Action(hero, &Hero::startMoveDown);
        keyUpActions[System::Windows::Forms::Keys::NumPad8] = gcnew System::Action(hero, &Hero::stopMoveUp);
        keyUpActions[System::Windows::Forms::Keys::NumPad5] = gcnew System::Action(hero, &Hero::stopMoveDown);
        keyUpActions[System::Windows::Forms::Keys::Space] = gcnew System::Action(hero, &Hero::shoot);

        Enemy^ enemy = gcnew Enemy(GenerateRandomPosition(), hero, gameObjects);
        enemy->GameOverEvent += gcnew Enemy::GameOverEventHandler(this, &GameForm::GameOver);
        enemy->EnemyDestroyedEvent += gcnew Enemy::EnemyDestroyedEventHandler(this, &GameForm::OnEnemyDestroyed);
        gameObjects->Add(enemy);

        enemyGenTimer->Start();
        timer->Start();

        this->BackColor = System::Drawing::Color::Black;
        Invalidate();
    }


    void GameForm::GenerateEnemyOrBox(System::Object^ sender, System::EventArgs^ e)
    {
        System::Drawing::Point randomPosition = GenerateRandomPosition();
        while (CheckCollision(randomPosition, 100, 100)) {
            randomPosition = GenerateRandomPosition();
        }

        if ((randomPosition.Y % 3 == 0) || (randomPosition.Y % 3 == 1)) {
            Enemy^ enemy = gcnew Enemy(randomPosition, hero, gameObjects);
            enemy->GameOverEvent += gcnew Enemy::GameOverEventHandler(this, &GameForm::GameOver);
            enemy->EnemyDestroyedEvent += gcnew Enemy::EnemyDestroyedEventHandler(this, &GameForm::OnEnemyDestroyed);
            gameObjects->Add(enemy);
        }
        else {
            Box^ box = gcnew Box(randomPosition, hero, gameObjects);
            box->GameOverEvent += gcnew Box::GameOverEventHandler(this, &GameForm::GameOver);
            box->BoxDestroyedEvent += gcnew Box::BoxDestroyedEventHandler(this, &GameForm::OnBoxDestroyed);
            gameObjects->Add(box);
        }
    }

    void GameForm::OnEnemyDestroyed()
    {
        score++;
        Invalidate();
    }


    void GameForm::OnBoxDestroyed()
    {
        boxesDestroyed++;
        hero->heal(10);  // Лечим героя за уничтожение ящика

        if (boxesDestroyed == TARGETS_TO_WIN)
            GameWin();

        Invalidate();
    }


    void GameForm::GameLoop(System::Object^ sender, System::EventArgs^ e)
    {
        for (int i = 0; i < gameObjects->Count; i++) {
            gameObjects[i]->act(); 
        }

        Invalidate(); 
    }
    int GameForm::CompareHighScores(System::Collections::Generic::KeyValuePair<System::String^, int> a, System::Collections::Generic::KeyValuePair<System::String^, int> b)
    {
        return b.Value.CompareTo(a.Value); // Сортировка по убыванию
    }

    void GameForm::OnPaint(System::Object^ obj, System::Windows::Forms::PaintEventArgs^ e)
    {
        System::Drawing::Graphics^ g = e->Graphics;


        for (int i = 0; i < gameObjects->Count; i++) {
            gameObjects[i]->draw(g);
        }


        System::Drawing::Font^ font = gcnew System::Drawing::Font("Comic Sans MS", 14, System::Drawing::FontStyle::Bold);

  
        System::Drawing::Brush^ defaultBrush = System::Drawing::Brushes::White;
        System::Drawing::Brush^ hpBrush;
        System::Drawing::Brush^ boxesDestroyedBrush;
        System::Drawing::Brush^ controlsBrush = System::Drawing::Brushes::Yellow;


        if (hero->getHeroHP() > 50) {
            hpBrush = System::Drawing::Brushes::Green; 
        }
        else if (hero->getHeroHP() > 20) {
            hpBrush = System::Drawing::Brushes::Yellow;
        }
        else {
            hpBrush = System::Drawing::Brushes::Red;
        }


        if (boxesDestroyed < TARGETS_TO_WIN - 2) {
            boxesDestroyedBrush = System::Drawing::Brushes::White;
        }
        else if (boxesDestroyed < TARGETS_TO_WIN) {
            boxesDestroyedBrush = System::Drawing::Brushes::Blue;
        }
        else {
            boxesDestroyedBrush = System::Drawing::Brushes::Firebrick;
        }

   
        g->DrawString("HP: " + hero->getHeroHP().ToString(), font, hpBrush, 100, 730);
        g->DrawString("Enemies killed: " + score.ToString(), font, defaultBrush, 1200, 730);
        g->DrawString("Targets destroyed: " + boxesDestroyed.ToString() + " of 5", font, boxesDestroyedBrush, 650, 730); 

        // Отображаем таблицу рекордов (топ-5)
        System::Drawing::Brush^ highScoreBrush = System::Drawing::Brushes::Yellow;
        g->DrawString("Top 5 High Scores:", font, highScoreBrush, 1200, 0);

        // Преобразуем словарь highScores в отсортированный список
        auto highScoresSorted = gcnew System::Collections::Generic::List<System::Collections::Generic::KeyValuePair<System::String^, int>>(highScores);
        highScoresSorted->Sort(gcnew System::Comparison<System::Collections::Generic::KeyValuePair<System::String^, int>>(this, &GameForm::CompareHighScores));


        int yOffset = 30; 
        for (int i = 0; i < System::Math::Min(5, highScoresSorted->Count); i++) {
            auto entry = highScoresSorted[i];
            g->DrawString((i + 1).ToString() + ". " + entry.Key + ": " + entry.Value.ToString(), font, highScoreBrush, 1200, yOffset);
            yOffset += 30;
        }

        // Инструкции для управления
        g->DrawString("Controls: ", font, controlsBrush, 100, 755);
        g->DrawString("SPACE to shoot ", font, controlsBrush, 220, 755);
        g->DrawString("NUM8, NUM5 to move ", font, controlsBrush, 400, 755);
        g->DrawString("R to restart ", font, controlsBrush, 660, 755);
        g->DrawString("P to pause ", font, controlsBrush, 830, 755);
        g->DrawString("ESC/F to toggle full screen", font, controlsBrush, 1000, 755);


        System::Drawing::Pen^ borderPen = gcnew System::Drawing::Pen(System::Drawing::Color::White, 3);  // Белая линия толщиной 3 пикселя
        g->DrawRectangle(borderPen, 0, 0, GAME_WIDTH, GAME_HEIGHT - 10);
    }




    void GameForm::OnKeyDown(System::Object^ obj, System::Windows::Forms::KeyEventArgs^ e)
    {
        if (keyDownActions->ContainsKey(e->KeyCode))
        {
            keyDownActions[e->KeyCode]->Invoke();
        }

        Invalidate();
    }

    void GameForm::OnKeyUp(System::Object^ obj, System::Windows::Forms::KeyEventArgs^ e)
    {
        if (keyUpActions->ContainsKey(e->KeyCode))
        {
            keyUpActions[e->KeyCode]->Invoke();
        }

        Invalidate();
    }

    // Вход в полноэкранный режим
    void GameForm::EnterFullScreen()
    {
        this->WindowState = System::Windows::Forms::FormWindowState::Maximized; 
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::None; 
        this->TopMost = true;  
        this->KeyPreview = true; 
    }

    // Выход из полноэкранного режима
    void GameForm::ExitFullScreen()
    {
        this->WindowState = System::Windows::Forms::FormWindowState::Normal;  
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Sizable;
        this->TopMost = false;  
    }

    // Переключение паузы
    void GameForm::TogglePause()
    {
        gamePaused = !gamePaused; 

        if (gamePaused)
        {
            enemyGenTimer->Stop();
            timer->Stop(); 
            pauseLabel->Visible = true; 
        }
        else
        {
            enemyGenTimer->Start(); 
            timer->Start(); 
            pauseLabel->Visible = false; 
        }

        this->Invalidate();
    }


    
    void GameForm::InitializeComponent(void)
    {
        
        this->components = gcnew System::ComponentModel::Container();
        this->Size = System::Drawing::Size(GAME_WIDTH, GAME_HEIGHT);  
        this->Text = L"Shooter Game";  
        this->BackColor = System::Drawing::Color::Black;  
        this->Padding = System::Windows::Forms::Padding(0);  
        this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
        this->KeyPreview = true;  

   
        this->pauseLabel = (gcnew System::Windows::Forms::Label());
        this->pauseLabel->AutoSize = true;
        this->pauseLabel->Location = System::Drawing::Point(GAME_WIDTH/2, GAME_HEIGHT/2);
        this->pauseLabel->Text = L"Game Paused";
        this->pauseLabel->Visible = false;
        this->pauseLabel->ForeColor = System::Drawing::Color::Red; 
        this->pauseLabel->Font = gcnew System::Drawing::Font("Comic Sans MS", 26, System::Drawing::FontStyle::Bold);
        this->Controls->Add(this->pauseLabel);

       
        this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &GameForm::OnPaint);
        this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &GameForm::OnKeyDown);
        this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &GameForm::OnKeyUp);

        // Включение двойной буферизации для предотвращения мерцания
        this->DoubleBuffered = true;
    }
}