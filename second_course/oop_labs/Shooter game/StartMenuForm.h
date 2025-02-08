#pragma once
#include "GameForm.h"

namespace ShooterGame {

    public ref class StartMenuForm : public System::Windows::Forms::Form
    {
    public:
        StartMenuForm();
        ~StartMenuForm();

    private:
        System::Windows::Forms::Button^ playButton;
        System::Windows::Forms::TextBox^ nicknameInput;
        System::Windows::Forms::Label^ nicknameLabel;
        System::Windows::Forms::ListView^ highScoresList; // Таблица для рекордов
        System::Windows::Forms::Label^ titleLabel;

        System::Collections::Generic::Dictionary<System::String^, int>^ highScores;

        void InitializeComponent();
        void LoadHighScores();
        void SaveHighScores();
        void PlayButton_Click(System::Object^ sender, System::EventArgs^ e);
    };
}
