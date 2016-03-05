#include "MainWindow.h"
#include "App.h"

MainWindow :: MainWindow(App* app):
    m_pApp(app)
{
    m_UI.setupUi(this);
}

MainWindow :: ~MainWindow()
{
    
}

void MainWindow :: closeEvent(QCloseEvent* ev)
{
}


