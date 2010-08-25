#include "Controller.h"
#include "ErrorHandling.h"
#include "Game.h"
#include "Logger.h"
#include "TetrisElement.h"
#include "BlockMover.h"
#include "GameStateNode.h"
#include "Player.h"
#include "TimedGame.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/Window.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"



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


        //
        // Get the Game object.
        //
        mThreadSafeGame.reset(new Protected<Game>(std::auto_ptr<Game>(new Game(mTetrisComponent->getNumRows(), mTetrisComponent->getNumColumns()))));


        //
        // Enable gravity.
        //
        mTimedGame.reset(new TimedGame(*mThreadSafeGame));
        mTimedGame->start();
        mTimedGame->setLevel(0);


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
            ScopedConstAtom<Game> rgame(*mThreadSafeGame);
            const Game & game = *(rgame.get());
            outGrid = game.currentNode()->state().grid();
            outActiveBlock = game.activeBlock();
            game.getFutureBlocks(mTetrisComponent->getNumFutureBlocks() + 1, outFutureBlockTypes);
        }
    }


    bool Controller::move(TetrisComponent * tetrisComponent, Direction inDirection)
    {
        if (mComputerPlayer || mBlockMover)
        {
            return false;
        }
        ScopedAtom<Game> wgame(*mThreadSafeGame);
        Game & game = *wgame.get();
        return game.move(inDirection);
    }


    void Controller::drop(TetrisComponent * tetrisComponent)
    {
        if (mComputerPlayer || mBlockMover)
        {
            return;
        }
        ScopedAtom<Game> wgame(*mThreadSafeGame);
        Game & game = *wgame.get();
        game.drop();
    }


    bool Controller::rotate(TetrisComponent * tetrisComponent)
    {
        if (mComputerPlayer || mBlockMover)
        {
            return false;
        }
        ScopedAtom<Game> wgame(*mThreadSafeGame);
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


    Protected<Game> & Controller::threadSafeGame()
    {
        return *mThreadSafeGame;
    }


    void Controller::log(const std::string & inMessage)
    {
        if (mLoggingTextBox)
        {
            std::string timestamp = Poco::DateTimeFormatter::format(Poco::DateTime(), "%H:%M:%S ");
            mLoggingTextBox->setValue(MakeString() << mLoggingTextBox->getValue() << "\r\n" << timestamp << inMessage);
            //int lineCount = ::SendMessage(mLoggingTextBox->handle(), EM_GETLINECOUNT, 0, 0);
            //::SendMessage(mLoggingTextBox->handle(), EM_LINESCROLL, 0, lineCount);
        }
    }


    void Controller::startAI(Game & game, size_t inDepth)
    {
        Assert(!mComputerPlayer);
        if (!mComputerPlayer)
        {
            std::auto_ptr<GameStateNode> endNode = game.endNode()->clone();
            size_t blockOffset = 0;
            Assert(endNode->depth() >= game.currentNode()->depth());
            if (endNode->depth() > game.currentNode()->depth())
            {
                blockOffset = endNode->depth() - game.currentNode()->depth();
            }
            BlockTypes futureBlocks_tooMany;
            game.getFutureBlocks(inDepth + blockOffset, futureBlocks_tooMany);

            BlockTypes futureBlocks;
            for (size_t idx = blockOffset; idx != futureBlocks_tooMany.size(); ++idx)
            {
                futureBlocks.push_back(futureBlocks_tooMany[idx]);
            }
            Assert(endNode->depth() == game.endNode()->depth());
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

        ScopedAtom<Game> wgame(*mThreadSafeGame);
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
                LogInfo(MakeString() << "Failed to move the blocks in time. Switching to minimal search depth. Game end node: " << endNode->depth() << ", result begin node: " << resultNode->depth() << ".");
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


        // Do we have computer moves lined up?
        if (!game.currentNode()->children().empty())
        {
            // Is the block mover taking care of them?
            if (mBlockMover)
            {
                // Is yes, check if isn't stuck.
                if (mBlockMover->status() == BlockMover::Status_Blocked)
                {
                    // If it is stuck, then empthy the queue of lined up moves. They are lost :(
                    game.currentNode()->clearChildren();

                    // Lower the search depth so that we can quickly think of some new moves!
                    LogInfo("Couldn't move the block to where I wanted. Plan B: quickly think of a some new moves!");
                    mSearchDepth = cMinSearchDepth;
                    mBlockMover.reset();
                }
            }
            // If no then start the block mover.
            else
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
