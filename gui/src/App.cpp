#include "App.h"
#include "MainWindow.h"
using namespace std;

App :: App(int& argc, char* argv[]):
    QApplication(argc,argv),
    m_Args(argc, (const char**)argv),
    m_pWindow(kit::make_unique<MainWindow>(this))
{
    m_pWindow->show();
}

App :: ~App()
{
}

void App :: quit()
{
    QApplication::quit();
}

