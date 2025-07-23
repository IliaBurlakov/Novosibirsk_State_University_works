#include "StartMenuForm.h"
using namespace System;
using namespace ShooterGame;
[STAThreadAttribute]
int main()
{
    System::Windows::Forms::Application::EnableVisualStyles();
    System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);
    ShooterGame::StartMenuForm form;
    System::Windows::Forms::Application::Run(% form);
    return 0;
}

