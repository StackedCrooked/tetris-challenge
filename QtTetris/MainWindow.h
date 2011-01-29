#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtGui>
#include "TetrisWidget.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/MultiplayerGame.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onRestart();

private:
    void restart();

    enum {
        cPlayerCount = 2
    };

    Tetris::MultiplayerGame mMultiplayerGame;
    typedef boost::shared_ptr<Tetris::SimpleGame> SimpleGamePtr;
    std::vector<SimpleGamePtr> mTetrisPlayers;
    std::vector<TetrisWidget *> mTetrisWidgets;
    QPushButton * mSwitchButton;
    QPushButton * mRestartButton;
};


#endif // MAINWINDOW_H
