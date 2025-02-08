#include "StartMenuForm.h"
#include "GameForm.h"
#include <fstream>

namespace ShooterGame {

    StartMenuForm::StartMenuForm()
    {
        this->Text = "Shooter Game - Start Menu";
        this->Size = System::Drawing::Size(400, 600);
        this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;

        highScores = gcnew System::Collections::Generic::Dictionary<System::String^, int>();
        InitializeComponent();
        LoadHighScores();
    }

    StartMenuForm::~StartMenuForm() {}

    void StartMenuForm::InitializeComponent()
    {
        titleLabel = gcnew System::Windows::Forms::Label();
        titleLabel->Text = "Welcome to Shooter Game!";
        titleLabel->Font = gcnew System::Drawing::Font("Comic Sans MS", 16, System::Drawing::FontStyle::Bold);
        titleLabel->AutoSize = true;
        titleLabel->Location = System::Drawing::Point(50, 20);
        this->Controls->Add(titleLabel);

        nicknameLabel = gcnew System::Windows::Forms::Label();
        nicknameLabel->Text = "Enter your nickname:";
        nicknameLabel->Location = System::Drawing::Point(50, 100);
        nicknameLabel->AutoSize = true;
        this->Controls->Add(nicknameLabel);

        nicknameInput = gcnew System::Windows::Forms::TextBox();
        nicknameInput->Size = System::Drawing::Size(200, 20);
        nicknameInput->Location = System::Drawing::Point(50, 130);
        this->Controls->Add(nicknameInput);

        // Кнопка Play
        playButton = gcnew System::Windows::Forms::Button();
        playButton->Text = "PLAY";
        playButton->Location = System::Drawing::Point(50, 200);
        playButton->Size = System::Drawing::Size(150, 40);
        playButton->Click += gcnew System::EventHandler(this, &StartMenuForm::PlayButton_Click);
        this->Controls->Add(playButton);

        // Таблица рекордов
        highScoresList = gcnew System::Windows::Forms::ListView();
        highScoresList->View = System::Windows::Forms::View::Details;
        highScoresList->Columns->Add("Nickname", 150);
        highScoresList->Columns->Add("Score", 80);
        highScoresList->Location = System::Drawing::Point(50, 300);
        highScoresList->Size = System::Drawing::Size(300, 200);
        this->Controls->Add(highScoresList);
    }

    void StartMenuForm::LoadHighScores()
    {
        System::String^ filePath = "highscores.txt";
        if (System::IO::File::Exists(filePath))
        {
            for each (System::String ^ line in System::IO::File::ReadAllLines(filePath))
            {
                array<System::String^>^ parts = line->Split(',');
                if (parts->Length == 2)
                {
                    System::String^ nickname = parts[0];
                    int score = System::Int32::Parse(parts[1]);
                    highScores[nickname] = score;
                }
            }
        }

        // Заполнить таблицу
        highScoresList->Items->Clear();
        for each (System::Collections::Generic::KeyValuePair<System::String^, int> entry in highScores)
        {
            auto item = gcnew System::Windows::Forms::ListViewItem(entry.Key);
            item->SubItems->Add(entry.Value.ToString());
            highScoresList->Items->Add(item);
        }
    }

    void StartMenuForm::SaveHighScores()
    {
        System::String^ filePath = "highscores.txt";
        System::IO::StreamWriter^ writer = gcnew System::IO::StreamWriter(filePath);
        for each (System::Collections::Generic::KeyValuePair<System::String^, int> entry in highScores)
        {
            writer->WriteLine(entry.Key + "," + entry.Value);
        }
        writer->Close();
    }

    void StartMenuForm::PlayButton_Click(System::Object^ sender, System::EventArgs^ e)
    {
        if (nicknameInput->Text->Trim()->Length == 0)
        {
            System::Windows::Forms::MessageBox::Show("Please enter a nickname!");
            return;
        }

        GameForm^ game = gcnew GameForm();
        game->SetPlayerNickname(nicknameInput->Text);
        game->SetHighScores(highScores);
        this->Hide();
        game->ShowDialog();

        // Сохранить рекорды
        SaveHighScores();
        this->Show();
    }
}
