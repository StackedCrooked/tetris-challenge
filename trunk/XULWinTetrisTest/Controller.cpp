#include "Controller.h"
#include "Logger.h"
#include "TetrisElement.h"
#include "BlockMover.h"
#include "GameStateNode.h"
#include "Player.h"
#include "ThreadSafeGame.h"
#include "TimedGame.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/Window.h"


namespace Tetris
{

    template<class T>
    T * FindComponentById(XULWin::Element * inElement, const std::string & inId)
    {
        if (XULWin::Element * inEl = inElement->getElementById(inId))
        {
            return inEl->component()->downcast<T>();
        }
        return 0;
    }

    Controller::Controller(HINSTANCE hInstance) :
        mXULRunner(hInstance),
        mWindow(0),
        mTetrisComponent(0),
        mFPSTextBox(0),
        mBlockCountTextBox(0),        
        mMovesAheadTextBox(0),
        mPercentTextBox(0),
        mMaxTimeTextBox(0),
        mAIProgressMeter(0),
        mLoggingTextBox(0),
        mThreadSafeGame(),
        mTimedGame(),
        mRefreshTimer(),
        mBlockMover(),
        mComputerPlayer(),
        mQuit(false),
        mAIThinkingTime(1000)
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
        mWindow = rootElement->component()->downcast<XULWin::Window>();
        if (!mWindow)
        {
            throw std::runtime_error("Root element is not of type winodw.");
        }
                    

        //
        // Connect the logger to the logging text box.
        //
        if (mLoggingTextBox = rootElement->getElementById("loggingTextbox")->component()->downcast<XULWin::TextBox>())
        {
            boost::function<void(const std::string&)> logFunction = boost::bind(&Controller::log, this, _1);
            XULWin::ErrorReporter::Instance().setLogger(logFunction);
            Tetris::Logger::Instance().setHandler(logFunction);
        }


        //
        // Get the Tetris component.
        //
        if (!(mTetrisComponent = FindComponentById<Tetris::TetrisComponent>(rootElement.get(), "tetris0")))
        {
            throw std::runtime_error("The XUL document must contain a <tetris> element with id \"tetris0\".");
        }

        mTetrisComponent->setController(this);


        // Get the future game state component.
        if (!(mFutureTetrisComponent = FindComponentById<Tetris::TetrisComponent>(rootElement.get(), "tetris1")))
        {
            LogWarning("Did not find future tetris component.");
        }

        if (mFutureTetrisComponent)
        {
            mFutureTetrisComponent->setController(this);
        }


        //
        // Get the Game object.
        //
        mThreadSafeGame.reset(new ThreadSafeGame(std::auto_ptr<Game>(new Game)));


        //
        // Enable gravity.
        //
        mTimedGame.reset(new TimedGame(*mThreadSafeGame));
        mTimedGame->start();


        //
        // Enable keyboard listener for activating the AI.
        //
        mTetrisComponent->OnKeyboardPressed.connect(boost::bind(&Controller::processKey, this, _1));


        //
        // Get stats widgets
        //
        if (XULWin::Element * el = rootElement->getElementById("fpsTextBox"))
        {
            mFPSTextBox = el->component()->downcast<XULWin::TextBox>();
        }
        else
        {
            LogWarning("The fps textbox was not found in the XUL document.");
        }


        if (XULWin::Element * el = rootElement->getElementById("blockCountTextBox"))
        {
            mBlockCountTextBox = el->component()->downcast<XULWin::TextBox>();
        }
        else
        {
            LogWarning("The block counter textbox was not found in the XUL document.");
        }


        for (size_t idx = 0; idx != 4; ++idx)
        {
            if (XULWin::Element * el = rootElement->getElementById(MakeString() << "lines" << (idx + 1) << "TextBox"))
            {
                if (!(mLinesTextBoxes[idx] = el->component()->downcast<XULWin::TextBox>()))
                {
                    throw std::runtime_error("This is not a textbox!");
                }
            }
            else
            {
                LogWarning(MakeString() << "The lines x" << idx << "TextBox element was not found in the XUL document.");
            }
        }


        //
        // Get the 'Moves ahead' textbox.
        //
        if (XULWin::Element * el = rootElement->getElementById("movesAheadTextBox"))
        {
            if (!(mMovesAheadTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The 'moves ahead' textbox was not found in the XUL document.");
            }
        }


        if (XULWin::Element * el = rootElement->getElementById("percentTextBox"))
        {
            if (!(mPercentTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The 'percentTextBox' textbox was not found in the XUL document.");
            }
        }


        if (XULWin::Element * el = rootElement->getElementById("maxTimeTextBox"))
        {
            if (!(mMaxTimeTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The 'curTimeTextBox' textbox was not found in the XUL document.");
            }
        }
        


        //
        // Get the widget that shows the remaing time for the AI to make its decision.
        //
        if (XULWin::Element * el = rootElement->getElementById("computerAIProgressMeter"))
        {
            if (!(mAIProgressMeter = el->component()->downcast<XULWin::ProgressMeter>()))
            {
                LogWarning("The textbox displaying the remaing time until AI decision is missing.");
            }
        }


        //
        // Activate the stats updater.
        // The WinAPI::Timer is non-threaded and you can safely access the WinAPI in its callbacks.
        //
        mRefreshTimer.reset(new XULWin::WinAPI::Timer);
        mRefreshTimer->start(boost::bind(&Controller::onRefresh, this), 500);
    }


    Controller::~Controller()
    {
        mComputerPlayer.reset();
        mTimedGame.reset();
    }


    void Controller::run()
    {
        mWindow->showModal(XULWin::WindowPos_CenterInScreen);
    }

            
    void Controller::getGameState(TetrisComponent * tetrisComponent,
                                  Grid & outGrid,
                                  Block & outActiveBlock,
                                  BlockTypes & outFutureBlockTypes)
    {
        if (tetrisComponent == mTetrisComponent)
        {
            ReadOnlyGame rgame(*mThreadSafeGame);
            const Game & game = *(rgame.get());
            outGrid = game.currentNode()->state().grid();
            outActiveBlock = game.activeBlock();
            game.getFutureBlocks(mTetrisComponent->getNumFutureBlocks() + 1, outFutureBlockTypes);
        }
        else if (tetrisComponent == mFutureTetrisComponent)
        {
            ReadOnlyGame rgame(*mThreadSafeGame);
            const Game & game = *(rgame.get());
            if (!game.currentNode()->children().empty())
            {
                const Game & game = *(rgame.get());
                const GameStateNode & node = (**game.currentNode()->children().begin());
                outGrid = node.state().grid();
            }
        }
    }


    bool Controller::move(TetrisComponent * tetrisComponent, Direction inDirection)
    {
        WritableGame wgame(*mThreadSafeGame);
        Game & game = *wgame.get();
        return game.move(inDirection);
    }


    void Controller::drop(TetrisComponent * tetrisComponent)
    {
        WritableGame wgame(*mThreadSafeGame);
        Game & game = *wgame.get();
        game.drop();
    }


    bool Controller::rotate(TetrisComponent * tetrisComponent)
    {
        WritableGame wgame(*mThreadSafeGame);
        Game & game = *wgame.get();
        return game.rotate();
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
        // Currently unused because it's blocking.
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
        // Don't allow AI if there are still blocks in the queue
        if (ReadOnlyGame(*mThreadSafeGame)->currentNode()->children().empty())
        {
            // Start the block mover
            mBlockMover.reset(new BlockMover(*mThreadSafeGame));           
            
            std::auto_ptr<GameStateNode> currentNode;
            BlockTypes futureBlocks;

            // Critical section: fetch a clone of the current node and the list of future blocks
            {
                ReadOnlyGame rgame(*mThreadSafeGame);
                const Game & game = *(rgame.get());
                game.getFutureBlocks(3, futureBlocks);
                currentNode = game.currentNode()->clone();
            }
            
            // Prepare just a check
            int currentDepth = currentNode->depth();

            // Setup the player object
            Player p(currentNode, futureBlocks, 1000, 3);           
            ChildNodePtr resultNode = p.start(); // Waiting...

            // Just a check
            assert(currentDepth + 1 == resultNode->depth());

            // Critical section: apply results
            {
                WritableGame wgame(*mThreadSafeGame);
                Game & game = *(wgame.get());
                game.currentNode()->addChild(resultNode);
            }
        }
    }


    void Controller::onRefresh()
    {
        try
        {
            refresh();
        }
        catch(const std::exception & inException)
        {
            LogError(MakeString() << "Exception thrown during Controller::onRefresh. Details: " << inException.what());
        }
    }


    void Controller::refresh()
    {
        // Flush the logger.
        Logger::Instance().flush();
    
        if (mBlockCountTextBox || mMovesAheadTextBox || mFutureTetrisComponent) // requires locking, brr..
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

            for (size_t idx = 0; idx != 4; ++idx)
            {
                const GameState::Stats & stats = game->currentNode()->state().stats();
                if (mLinesTextBoxes[idx])
                {
                    mLinesTextBoxes[idx]->setValue(MakeString() << stats.numLines(idx));
                }
            }
        }

        if (mFPSTextBox)
        {
            mFPSTextBox->setValue(MakeString() << mTetrisComponent->getFPS());
        }

        if (mComputerPlayer)
        {
            int remainingTime = mComputerPlayer->remainingTimeMs();
            int maxTime = 0;
            {
                boost::mutex::scoped_lock lock(mAIThinkingTimeMutex);
                maxTime = mAIThinkingTime;
            }
            
            int curTime = maxTime - remainingTime;            
            int percentage = (100 * curTime) / maxTime;
            if (mAIProgressMeter)
            {
                mAIProgressMeter->setDisabled(false);
                mAIProgressMeter->setValue(percentage);
            }

            if (mPercentTextBox && mMaxTimeTextBox)
            {
                mPercentTextBox->setValue(MakeString() << percentage);
                mMaxTimeTextBox->setValue(MakeString() << maxTime / 1000);
            }
        }
        else
        {
            mAIProgressMeter->setDisabled(true);
            mAIProgressMeter->setValue(0);
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
