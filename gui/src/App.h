#ifndef APP_H_UFOSKFLO
#define APP_H_UFOSKFLO

#include <QtGui>
#include <QtWidgets>
#include "kit/args/args.h"
#include "MainWindow.h"

class App:
    public QApplication

{
    Q_OBJECT

    public:
        
        App(int& argc, char* argv[]);

        App(const App&) = default;
        App(App&&) = default;
        App& operator=(const App&) = default;
        App& operator=(App&&) = default;

        virtual ~App();

    private Q_SLOTS:
        
        void quit();
    
    private:
    
        Args m_Args;
        std::unique_ptr<MainWindow> m_pWindow;
};

#endif
