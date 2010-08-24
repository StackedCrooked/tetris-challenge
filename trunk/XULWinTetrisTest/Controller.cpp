#include "Controller.h"
#include "ErrorHandling.h"
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
        mComputerEnabledCheckBox(0),
        mStatusTextBox(0),
        mMovesAheadTextBox(0),
        mSearchDepthTextBox(0),
        mPercentTextBox(0),
        mMaxTimeTextBox(0),
        mLoggingTextBox(0),
        mThreadSafeGame(),
        mTimedGame(),
        mRefreshTimer(),
        mComputerPlayer(),
        mBlockMover(),
        mSearchDepth(cMinSearchDepth),
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
        mThreadSafeGame.reset(new ThreadSafeGame(std::auto_ptr<Game>(new Game(5, 10))));


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


        if (XULWin::Element * el = rootElement->getElementById("computerEnabled"))
        {
            if (!(mComputerEnabledCheckBox = el->component()->downcast<XULWin::CheckBox>()))
            {
                LogWarning("The 'computerEnabled' checkbox was not found in the XUL document.");
            }
        }


        if (XULWin::Element * el = rootElement->getElementById("movesAheadTextBox"))
        {
            if (!(mMovesAheadTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The 'moves ahead' textbox was not found in the XUL document.");
            }
        }


        if (XULWin::Element * el = rootElement->getElementById("searchDepthTextBox"))
        {
            if (!(mSearchDepthTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The 'search depth' textbox was not found in the XUL document.");
            }
        }


        if (XULWin::Element * el = rootElement->getElementById("statusTextBox"))
        {
            if (!(mStatusTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The 'status' textbox was not found in the XUL document.");
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
        // Activate the stats updater.
        // The WinAPI::Timer is non-threaded and you can safely access the WinAPI in its callbacks.
        //
        mRefreshTimer.reset(new XULWin::WinAPI::Timer);
        mRefreshTimer->start(boost::bind(&Controller::onRefresh, this), 10);
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
        if (mComputerPlayer || mBlockMover)
        {
            return false;
        }
        WritableGame wgame(*mThreadSafeGame);
        Game & game = *wgame.get();
        return game.move(inDirection);
    }


    void Controller::drop(TetrisComponent * tetrisComponent)
    {
        if (mComputerPlayer || mBlockMover)
        {
            return;
        }
        WritableGame wgame(*mThreadSafeGame);
        Game & game = *wgame.get();
        game.drop();
    }


    bool Controller::rotate(TetrisComponent * tetrisComponent)
    {
        if (mComputerPlayer || mBlockMover)
        {
            return false;
        }
        WritableGame wgame(*mThreadSafeGame);
        Game & game = *wgame.get();
        return game.rotate();
    }


    void Controller::setQuitFlag()
    {
        mQuit = true;
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


    void Controller::startAI(Game & game, size_t inDepth)
    {
        Assert(!mComputerPlayer);
        if (!mComputerPlayer)
        {
            std::auto_ptr<GameStateNode> endNode = game.endNode()->clone();
            size_t blockOffset = endNode->depth() - game.currentNode()->depth();
            BlockTypes futureBlocks_tooMany;
            game.getFutureBlocks(inDepth + blockOffset, futureBlocks_tooMany);

            BlockTypes futureBlocks;
            for (size_t idx = blockOffset; idx != futureBlocks_tooMany.size(); ++idx)
            {
                futureBlocks.push_back(futureBlocks_tooMany[idx]);
            }
            mComputerPlayer.reset(new Player(endNode, futureBlocks));
            mComputerPlayer->start();
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

        WritableGame wgame(*mThreadSafeGame);
        Game & game = *(wgame.get());
        if (game.isGameOver())
        {
            mStatusTextBox->setValue("Game Over!");
            return;
        }

        // Check if the computer player has finished. If yes, then get the results.
        if (mComputerPlayer && mComputerPlayer->isFinished())
        {
            // Did we manage to think all our moves before the block was dropped?
            GameStateNode * endNode = game.endNode();
            ChildNodePtr resultNode = mComputerPlayer->result();
            if (resultNode->depth() == endNode->depth() + 1)
            {
                if (mSearchDepth < cMaxSearchDepth)
                {
                    mSearchDepth++;
                }
                endNode->addChild(resultNode);
            }
            else
            {
                LogWarning("Failed to move the blocks in time. Dropping to a lower ambition.");
                mSearchDepth = cMinSearchDepth;
            }
            mComputerPlayer.reset();
        }

                
        if (mComputerEnabledCheckBox->isChecked() && !mComputerPlayer)
        {
            GameStateNode * endNode = game.endNode();
            if (!endNode->state().isGameOver() &&
                endNode->depth() - game.currentNode()->depth() + mSearchDepth <=  3 * cMaxSearchDepth)
            {
                startAI(game, mSearchDepth);
            }
        }


        // Do we have computer moves lined up? If yes, then activate the Block mover.
        if (!game.currentNode()->children().empty())
        {
            if (!mBlockMover)
            {
                mBlockMover.reset(new BlockMover(*mThreadSafeGame));
            }
        }
        else
        {
            mBlockMover.reset();
        }
        

        if (mStatusTextBox)
        {
            std::string status;
            if (mComputerPlayer)
            {
                status = "Thinking";
            }
            
            if (mBlockMover)
            {
                if (!status.empty())
                {
                    status += " + ";
                }
                status += "Moving";
            }
            
            mStatusTextBox->setValue(status.empty() ? "Inactive" : status);
        }


        if (mBlockCountTextBox)
        {
            mBlockCountTextBox->setValue(MakeString() << game.currentBlockIndex());
        }


        if (mMovesAheadTextBox)
        {
            mMovesAheadTextBox->setValue(MakeString() << (game.endNode()->depth() - game.currentNode()->depth()) << "/" << 3 * cMaxSearchDepth);
        }

        if (mSearchDepthTextBox)
        {
            mSearchDepthTextBox->setValue(MakeString() << mSearchDepth);
        }

        for (size_t idx = 0; idx != 4; ++idx)
        {
            const GameState::Stats & stats = game.currentNode()->state().stats();
            if (mLinesTextBoxes[idx])
            {
                mLinesTextBoxes[idx]->setValue(MakeString() << stats.numLines(idx));
            }
        }

        if (mFPSTextBox)
        {
            mFPSTextBox->setValue(MakeString() << mTetrisComponent->getFPS());
        }
    }


    void Controller::processKey(int inKey)
    {
        // No interest.
    }


} // namespace Tetris
