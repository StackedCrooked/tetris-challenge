#ifndef NEWGAMEDIALOG_H
#define NEWGAMEDIALOG_H


#ifndef QT_NO_KEYWORDS
#define QT_NO_KEYWORDS
#endif

#ifndef QT_NO_DEBUG
#define QT_NO_DEBUG
#endif


#include <QtGui/QtGui>


class NewGameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewGameDialog(QWidget * inParent);

    virtual ~NewGameDialog();

    enum Selection
    {
        Selection_Human,
        Selection_Computer,
        Selection_HumanVsComputer,
        Selection_ComputerVsComputer
    };

    Selection selection() const;

private:
    QRadioButton * mNewHumanSinglePlayerGame;
    QRadioButton * mNewComputerSinglePlayerGame;
    QRadioButton * mNewHumanVsComputerMultiplayerGame;
    QRadioButton * mNewComputerVsComputerMultiplayerGame;
    QDialogButtonBox * mDialogButtonBox;
};


#endif // NEWGAMEDIALOG_H
