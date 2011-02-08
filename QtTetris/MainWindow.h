#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QtGui>
#include "TetrisWidget.h"
#include "Tetris/SimpleGame.h"
#include "Tetris/MultiplayerGame.h"
#include "Tetris/Player.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>


class MainWindow : public QWidget
{
    Q_OBJECT

public:
    static MainWindow * GetInstance();

    MainWindow(QWidget *parent = 0);

    ~MainWindow();

    virtual bool event(QEvent * inEvent);

    void logMessage(const std::string & inMessage);

private slots:
    void onRestart();

private:
    static MainWindow * sInstance;

private:
    void restart();

    std::vector<TetrisWidget *> mTetrisWidgets;
    QPushButton * mRestartButton;
    QTextEdit * mLogField;
    bool mRestartFlag;
};


#endif // MAINWINDOW_H
