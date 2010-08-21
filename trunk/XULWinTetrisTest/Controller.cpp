#include "Controller.h"
#include "Logger.h"
#include "TetrisComponent.h"
#include "TetrisElement.h"
#include "BlockMover.h"
#include "GameStateNode.h"
#include "Player.h"
#include "ThreadSafeGame.h"
#include "TimedGame.h"
#include "XULWin/Window.h"


namespace Tetris
{

    Controller::Controller(HINSTANCE hInstance) :
        mXULRunner(hInstance),
        mBlockCountTextBox(0),        
        mMovesAheadTextBox(0),
        mAIProgressMeter(0),
        mLoggingTextBox(0),
        mThreadSafeGame(),
        mTimedGame(),
        mRefreshTimer(),
        mBlockMover(),
        mAIThread(),
        mComputerPlayer(),
        mQuit(false)
    {

        //
        // Parse the XUL document.
        //
        XULWin::ElementPtr rootElement = mXULRunner.loadXULFromFile("XULWinTetrisTest.xul");
        if (!rootElement)
        {
            throw std::runtime_error("Failed to load the root element");
        }


        //
        // Get the Window component.
        //
        XULWin::Window * wnd = rootElement->component()->downcast<XULWin::Window>();
        if (!wnd)
        {
            throw std::runtime_error("Root element is not of type winodw.");
        }


        //
        // Get the Tetris component.
        //
        Tetris::TetrisComponent * tetrisComponent(0);
        std::vector<Tetris::TetrisElement *> tetrisElements;
        rootElement->getElementsByType<Tetris::TetrisElement>(tetrisElements);
        if (tetrisElements.empty())
        {
            throw std::runtime_error("The window does not contain any 'tetris' elements.");
        }
        else
        {
            tetrisComponent = tetrisElements[0]->component()->downcast<Tetris::TetrisComponent>();
        }

        if (tetrisElements.size())
        {
            LogWarning("Multiple tetris elements found in the XUL file. Only the first one can be used for AI.");
        }


        //
        // Get the Game object.
        //
        mThreadSafeGame.reset(new ThreadSafeGame(tetrisComponent->getThreadSafeGame()));
                    

        //
        // Connect the logger to the logging text box.
        //
        if (mLoggingTextBox = rootElement->getElementById("loggingTextbox")->component()->downcast<XULWin::TextBox>())
        {
            Tetris::Logger::Instance().setHandler(boost::bind(&Controller::log, this, _1));
        }

        //
        // Enable gravity.
        //
        mTimedGame.reset(new TimedGame(*mThreadSafeGame));
        mTimedGame->start();


        //
        // Enable keyboard listener for activating the AI.
        //
        tetrisComponent->OnKeyboardPressed.connect(boost::bind(&Controller::processKey, this, _1));


        //
        // Get stats widgets
        //
        if (!(mBlockCountTextBox = rootElement->getElementById("blockCountTextBox")->component()->downcast<XULWin::TextBox>()))
        {
            LogWarning("The block counter textbox was not found in the XUL document.");
        }


        //
        // Get the 'Moves ahead' textbox.
        //
        if (!(mMovesAheadTextBox = rootElement->getElementById("movesAheadTextBox")->component()->downcast<XULWin::TextBox>()))
        {
            LogWarning("The 'moves ahead' textbox was not found in the XUL document.");
        }


        //
        // Get the widget that shows the remaing time for the AI to make its decision.
        //
        if (!(mAIProgressMeter = rootElement->getElementById("computerAIProgressMeter")->component()->downcast<XULWin::ProgressMeter>()))
        {
            LogWarning("The textbox displaying the remaing time until AI decision is missing.");
        }


        //
        // Activate the stats updater.
        // The WinAPI::Timer is non-threaded and you can safely access the WinAPI in its callbacks.
        //
        mRefreshTimer.reset(new XULWin::WinAPI::Timer);
        mRefreshTimer->start(boost::bind(&Controller::refresh, this), 50);


        //
        // Show the Window.
        //
        wnd->showModal(XULWin::WindowPos_CenterInScreen);
    }


    Controller::~Controller()
    {
        mComputerPlayer.reset();
        mTimedGame.reset();
    }


    void Controller::setQuitFlag()
    {
        mQuit = true;
        if (mComputerPlayer)
        {
            mComputerPlayer->stop();
        }
    }


    void Controller::joinAllThreads()
    {
        if (mAIThread)
        {
            mAIThread->join();
        }
    }


    ThreadSafeGame & Controller::threadSafeGame()
    {
        return *mThreadSafeGame;
    }


    void Controller::log(const std::string & inMessage)
    {
        if (mLoggingTextBox)
        {
            mLoggingTextBox->setValue(inMessage + "\r\n" + mLoggingTextBox->getValue());
        }
    }


    void Controller::startAI()
    {
        if (!mAIThread)
        {
            mAIThread.reset(new boost::thread(boost::bind(&Controller::precalculate, this)));
        }
        else
        {
            LogWarning("The AI thread has already started.");
        }
    }


    void Controller::refresh()
    {
        // Flush the logger.
        Logger::Instance().flush();
        if (mBlockCountTextBox || mMovesAheadTextBox)
        {
            // Only update every 500 ms to reduce the locking of the Game object.
            static Poco::Stopwatch fStopwatch;
            fStopwatch.start();
            if (1000 * fStopwatch.elapsed() / fStopwatch.resolution() >= 500)
            {
                ReadOnlyGame game(*mThreadSafeGame);
                if (mBlockCountTextBox)
                {
                    mBlockCountTextBox->setValue(MakeString() << game->currentBlockIndex());
                }

                if (mMovesAheadTextBox)
                {
                    int countMovesAhead = 0;
                    const GameStateNode * tmp = game->currentNode();
                    while (!tmp->children().empty())
                    {
                        tmp = tmp->children().begin()->get();
                        countMovesAhead++;
                    }
                    mMovesAheadTextBox->setValue(MakeString() << countMovesAhead);
                }
                fStopwatch.restart();
            }
        }

        if (mAIProgressMeter)
        {
            if (mComputerPlayer)
            {
                mAIProgressMeter->setDisabled(false);
                int percentage = 100 *(cAIThinkingTime - mComputerPlayer->remainingTimeMs()) / cAIThinkingTime;
                mAIProgressMeter->setValue(percentage);
            }
            else
            {
                mAIProgressMeter->setDisabled(true);
                mAIProgressMeter->setValue(0);
            }
        }            
    }

        
    void Controller::precalculate()
    {
        // Thead entry point
        try
        {
            // Start the block mover
            mBlockMover.reset(new BlockMover(*mThreadSafeGame));

            // Infinite AI
            while (!mQuit)
            {
                BlockTypes blockTypes;
                std::auto_ptr<GameStateNode> clonedStartingNode;
                int depthOfStartingNode = 0;

                // Critical section:
                //   - clone the current GameState object
                //   - fetch future blocks
                {
                    WritableGame game(*mThreadSafeGame);
                    GameStateNode * startingNode = game->currentNode();
                    while (!startingNode->children().empty())
                    {
                        GameStateNode & node = **startingNode->children().begin();
                        startingNode = &node;
                    }
                    clonedStartingNode = startingNode->clone();
                    depthOfStartingNode = clonedStartingNode->depth() - game->currentNode()->depth();

                    BlockTypes tooManyBlockTypes;
                    game->getFutureBlocks(depthOfStartingNode + cAIMaxDepth, tooManyBlockTypes);
                    for (size_t idx = depthOfStartingNode; idx != tooManyBlockTypes.size(); ++idx)
                    {
                        blockTypes.push_back(tooManyBlockTypes[idx]);
                    }
                }

                // Blocking until best path found.
                calculateOptimalPath(clonedStartingNode, blockTypes);
            }
        }
        catch (const std::exception & inException)
        {
            LogError(inException.what()); // TODO: Make thread safe!!!
        }
    }


    void Controller::calculateOptimalPath(std::auto_ptr<GameStateNode> inClonedGameState, const BlockTypes & inBlockTypes)
    {
        //
        // Start the number crunching. This will take a while to complete (max cAIThinkingTime).
        //
        int currentGameDepth = inClonedGameState->depth();
        mComputerPlayer.reset(new Player(inClonedGameState, inBlockTypes, cAIThinkingTime, cAIMaxDepth));
        mComputerPlayer->start(); // Wait for results... (limited by cAIThinkingTime)


        //
        // Eliminate the inferior branches. The tree will be reduced to a path of nodes defining
        // the best game strategy.
        //
        ChildNodePtr bestLastChild = mComputerPlayer->getBestChild();
        GameStateNode * bestFirstChild = bestLastChild.get();
        while (bestFirstChild->depth() > currentGameDepth + 1)
        {
            bestFirstChild = bestFirstChild->parent();
        }
        DestroyInferiorChildren(bestFirstChild, bestLastChild.get());
        

        //
        // We actually don't know yet which of our first generation children leads to the best
        // last child. This little loop does the searching.
        //
        ChildNodePtr firstChild;
        for (ChildNodes::iterator it = mComputerPlayer->node()->children().begin(); it != mComputerPlayer->node()->children().end(); ++it)
        {
            ChildNodePtr child = *it;
            if (child.get() == bestFirstChild)
            {
                firstChild = child;
                break;
            }
        }

        // If we didn't find the first child, then something is wrong with our code. Unforgivable.
        if (!firstChild)
        {
            throw std::logic_error("The AI result-path does not include one of the first generation child nodes.");
        }
        
        // Critical section: append the first child to the currently last child
        {
            WritableGame game(*mThreadSafeGame);
            GameStateNode * lastChild = game->currentNode();
            while (!lastChild->children().empty())
            {
                lastChild = lastChild->children().begin()->get();
            }
            lastChild->children().insert(firstChild);
        }
    }


    void Controller::processKey(int inKey)
    {
        if (inKey == VK_DELETE)
        {
            startAI();
        }
    }


} // namespace Tetris
