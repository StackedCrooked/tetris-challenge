#include "NewGameDialog.h"
#include <stdexcept>


NewGameDialog::NewGameDialog(QWidget *inParent) :
    QDialog(inParent, Qt::Dialog),
    mNewHumanSinglePlayerGame(0),
    mNewComputerSinglePlayerGame(0),
    mNewHumanVsComputerMultiplayerGame(0),
    mNewComputerVsComputerMultiplayerGame(0),
    mDialogButtonBox(0)
{
    this->setWindowTitle("New");
    setLayout(new QVBoxLayout);

    QGroupBox * groupBox = new QGroupBox("Start a new game:", this);
    mNewHumanSinglePlayerGame = new QRadioButton("Human", groupBox);
    mNewComputerSinglePlayerGame = new QRadioButton("Computer", groupBox);
    mNewHumanVsComputerMultiplayerGame = new QRadioButton("Human vs Computer", groupBox);
    mNewComputerVsComputerMultiplayerGame = new QRadioButton("Computer vs Computer", groupBox);
    mNewHumanSinglePlayerGame->setChecked(true);

    QVBoxLayout * groupLayout = new QVBoxLayout;
    groupLayout->addWidget(mNewHumanSinglePlayerGame);
    groupLayout->addWidget(mNewComputerSinglePlayerGame);
    groupLayout->addWidget(mNewHumanVsComputerMultiplayerGame);
    groupLayout->addWidget(mNewComputerVsComputerMultiplayerGame);

    groupBox->setLayout(groupLayout);
    layout()->addWidget(groupBox);

    mDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect(mDialogButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mDialogButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mDialogButtonBox->button(QDialogButtonBox::Ok)->setDefault(true);
    layout()->addWidget(mDialogButtonBox);

    setTabOrder(groupBox, mDialogButtonBox);
    groupBox->setFocus();
}


NewGameDialog::~NewGameDialog()
{
}


NewGameDialog::Selection NewGameDialog::selection() const
{
    if (mNewHumanSinglePlayerGame->isChecked())
    {
        return Selection_Human;
    }
    else if (mNewComputerSinglePlayerGame->isChecked())
    {
        return Selection_Computer;
    }
    else if (mNewHumanVsComputerMultiplayerGame->isChecked())
    {
        return Selection_HumanVsComputer;
    }
    else if (mNewComputerVsComputerMultiplayerGame->isChecked())
    {
        return Selection_ComputerVsComputer;
    }
    else
    {
        throw std::logic_error("No radio button is selected.");
    }
}
