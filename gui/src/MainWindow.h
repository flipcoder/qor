#ifndef WINDOW_H_LQPPOFKN
#define WINDOW_H_LQPPOFKN

#include <QMainWindow>
#include "ui/ui_player.h"

class App;

class MainWindow:
    public QMainWindow
{
    Q_OBJECT
        
    public:

        MainWindow(App* app);
        virtual ~MainWindow();

        MainWindow(const MainWindow&) = default;
        MainWindow(MainWindow&&) = default;
        MainWindow& operator=(const MainWindow&) = default;
        MainWindow& operator=(MainWindow&&) = default;

        App* m_pApp;
        Ui::MainWindow m_UI;

    protected:

        void closeEvent(QCloseEvent* ev);
};

#endif
