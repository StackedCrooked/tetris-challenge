#include "Controller.h"
#include "Tetris/ErrorHandling.h"
#include "Tetris/Game.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/Logger.h"
#include "TetrisElement.h"
#include "Tetris/BlockMover.h"
#include "Tetris/GameStateNode.h"
#include "Tetris/Player.h"
#include "Tetris/TimedGame.h"
#include "XULWin/Conversions.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/Menu.h"
#include "XULWin/Window.h"
#include "XULWin/WinUtils.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"
#include <boost/lexical_cast.hpp>


namespace Tetris
{

    Controller::Controller(HINSTANCE hInstance) :
        mXULRunner(hInstance),
        mRootElement(),
        mWindow(0),
        mAboutDialogRootElement(),
        mTetrisComponent(0),
        mFPSTextBox(0),
        mBlockCountTextBox(0),        
        mLevelTextBox(0),
        mScoreTextBox(0),
        mComputerEnabledCheckBox(0),
        mSearchDepth(0),
        mMovementSpeed(0),
        mStatusTextBox(0),
        mMovesAheadTextBox(0),
        mStrategiesMenuList(0),
        mGameHeightFactor(0),
        mLastBlockHeightFactor(0),
        mNumHolesFactor(0),
        mNumLinesFactor(0),
        mNumSinglesFactor(0),
        mNumDoublesFactor(0),
        mNumTriplesFactor(0),
        mNumTetrisesFactor(0),
        mGameStateScore(0),
        mLoggingTextBox(0),
        mProtectedGame(),
        mTimedGame(),
        mRefreshTimer(),
        mComputerPlayer(),
        mBlockMover(),
        mEvaluator(new Balanced),
        mQuit(false)
    {
        //
        // Parse the XUL document.
        //
        mRootElement = mXULRunner.loadApplication("application.ini");
        if (!mRootElement)
        {
            throw std::runtime_error("Failed to load the root element");
        }


        //
        // Get the Window component.
        //
        mWindow = mRootElement->component()->downcast<XULWin::Window>();
        if (!mWindow)
        {
            throw std::runtime_error("Root element is not of type winodw.");
        }
                    

		// Disabled XULWin logging during constructor. We have our own logger.
        XULWin::ErrorCatcher catcher;
        catcher.disableLogging(true);

        //
        // Connect the logger to the logging text box.
        //
        if (XULWin::Element * el = mRootElement->getElementById("loggingTextbox"))
        {
            if (mLoggingTextBox = el->component()->downcast<XULWin::TextBox>())
            {
                boost::function<void(const std::string&)> logFunction = boost::bind(&Controller::log, this, _1);
                XULWin::ErrorReporter::Instance().setLogger(logFunction);
                Tetris::Logger::Instance().setHandler(logFunction);
            }
        }


        //
        // Get the Tetris component.
        //
        if (!(mTetrisComponent = findComponentById<Tetris::TetrisComponent>("tetris0")))
        {
            throw std::runtime_error("The XUL document must contain a <tetris> element with id \"tetris0\".");
        }

        mTetrisComponent->setController(this);


        //
        // Get the Game object.
        //
        mProtectedGame.reset(new Protected<Game>(std::auto_ptr<Game>(new Game(mTetrisComponent->getNumRows(), mTetrisComponent->getNumColumns()))));


        //
        // Enable gravity.
        //
        mTimedGame.reset(new TimedGame(*mProtectedGame));


        //
        // Enable keyboard listener for activating the AI.
        //
        mTetrisComponent->OnKeyboardPressed.connect(boost::bind(&Controller::processKey, this, _1));


        //
        // Get stats widgets
        //
        if (XULWin::Element * el = mRootElement->getElementById("fpsTextBox"))
        {
            mFPSTextBox = el->component()->downcast<XULWin::TextBox>();
        }
        else
        {
            LogWarning("The fps textbox was not found in the XUL document.");
        }


        if (XULWin::Element * el = mRootElement->getElementById("blockCountTextBox"))
        {
            mBlockCountTextBox = el->component()->downcast<XULWin::TextBox>();
        }
        else
        {
            LogWarning("The block counter textbox was not found in the XUL document.");
        }


        for (size_t idx = 0; idx != 4; ++idx)
        {
            if (XULWin::Element * el = mRootElement->getElementById(MakeString() << "lines" << (idx + 1) << "TextBox"))
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
        

        if (XULWin::Element * el = mRootElement->getElementById("levelTextBox"))
        {
            if (!(mLevelTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The element with id 'levelTextBox' was found but it was not of type 'textbox'.");
            }
        }


        if (XULWin::Element * el = mRootElement->getElementById("scoreTextBox"))
        {
            if (!(mScoreTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The element with id 'scoreTextBox' was found but it was not of type 'textbox'.");
            }
        }


        if (XULWin::Element * el = mRootElement->getElementById("computerEnabled"))
        {
            if (!(mComputerEnabledCheckBox = el->component()->downcast<XULWin::CheckBox>()))
            {
                LogWarning("The element with id 'computerEnabled' was found but it was not of type 'checkbox'.");
            }            
        }

        if (mSearchDepth = findComponentById<XULWin::SpinButton>("searchDepth"))
        {
            XULWin::WinAPI::SpinButton_SetRange(mSearchDepth->handle(), 1, 4);
        }
        
        
        if (mMovementSpeed = findComponentById<XULWin::SpinButton>("movementSpeed"))
        {            
            XULWin::WinAPI::SpinButton_SetRange(mMovementSpeed->handle(), 1, 100);
        }


        if (XULWin::Element * el = mRootElement->getElementById("movesAheadTextBox"))
        {
            if (!(mMovesAheadTextBox = el->component()->downcast<XULWin::TextBox>()))
            {
                LogWarning("The 'moves ahead' textbox was not found in the XUL document.");
            }
        }


        if (XULWin::Element * el = mRootElement->getElementById("strategiesMenuList"))
        {
            if (!(mStrategiesMenuList = el->component()->downcast<XULWin::MenuList>()))
            {
                LogWarning("The 'strategies' menulist was not found in the XUL document.");
            }
            else
            {
                mScopedEventListener.connect(el, boost::bind(&Controller::onStrategySelected, this, _1, _2));
                if (XULWin::Element * xmlpopup = mRootElement->getElementById("strategiesPopup"))
                {
                    // Populate it
                    XULWin::AttributesMapping attr;
                    attr["label"] = "Balanced";
                    attr["id"] = "balancedStrategy";
                    XULWin::ElementPtr menuItem = XULWin::XMLMenuItem::Create(xmlpopup, attr);

                    attr["id"] = "perfectionisticStrategy";
                    attr["label"] = "Perfectionistic";
                    XULWin::XMLMenuItem::Create(xmlpopup, attr);

                    attr["id"] = "scoreOnlyStrategy";
                    attr["label"] = "Score";
                    XULWin::XMLMenuItem::Create(xmlpopup, attr);
                }
            }
        }

        mGameHeightFactor = findComponentById<XULWin::SpinButton>("gameHeightFactor");
        mLastBlockHeightFactor = findComponentById<XULWin::SpinButton>("lastBlockHeightFactor");
        mNumHolesFactor = findComponentById<XULWin::SpinButton>("numHolesFactor");
        mNumLinesFactor = findComponentById<XULWin::SpinButton>("numLinesFactor");
        mNumSinglesFactor = findComponentById<XULWin::SpinButton>("numSinglesFactor");
        mNumDoublesFactor = findComponentById<XULWin::SpinButton>("numDoublesFactor");
        mNumTriplesFactor = findComponentById<XULWin::SpinButton>("numTriplesFactor");
        mNumTetrisesFactor = findComponentById<XULWin::SpinButton>("numTetrisesFactor");
        mGameStateScore = findComponentById<XULWin::TextBox>("gameStateScore");
        mStatusTextBox = findComponentById<XULWin::TextBox>("statusTextBox");

        if (XULWin::MenuItem * newMenuItem = findComponentById<XULWin::MenuItem>("newMenuItem"))
        {
            mScopedEventListener.connect(newMenuItem->el(), boost::bind(&Controller::onNew, this, _1, _2));
        }

        if (XULWin::MenuItem * quitMenuItem = findComponentById<XULWin::MenuItem>("quitMenuItem"))
        {
            mScopedEventListener.connect(quitMenuItem->el(), boost::bind(&Controller::onQuit, this, _1, _2));
        }

        if (XULWin::MenuItem * aboutMenuItem = findComponentById<XULWin::MenuItem>("aboutMenuItem"))
        {
            mScopedEventListener.connect(aboutMenuItem->el(), boost::bind(&Controller::onAboutMenuItem, this, _1, _2));
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
        mBlockMover.reset();
    }


    LRESULT Controller::onNew(WPARAM wParam, LPARAM lParam)
    {
        mComputerPlayer.reset();
        mTimedGame.reset();
        mBlockMover.reset();
        mProtectedGame.reset(new Protected<Game>(std::auto_ptr<Game>(new Game(mTetrisComponent->getNumRows(), mTetrisComponent->getNumColumns()))));
        mTimedGame.reset(new TimedGame(*mProtectedGame));
        return XULWin::cHandled;
    }


    LRESULT Controller::onQuit(WPARAM wParam, LPARAM lParam)
    {
        ::PostQuitMessage(0);
        return XULWin::cHandled;
    }


    LRESULT Controller::onAboutMenuItem(WPARAM wParam, LPARAM lParam)
    {
        if (!(mAboutDialogRootElement = mXULRunner.loadXULFromFile("chrome/content/AboutDialog.xul")))
        {
            throw std::runtime_error("Did not find the AboutDialog.xul file.");
        }

        if (XULWin::Dialog * dialog = mAboutDialogRootElement->component()->downcast<XULWin::Dialog>())
        {
            if (XULWin::Element * okButton = mAboutDialogRootElement->getElementById("okButton"))
            {
                XULWin::ScopedEventListener dialogEventListener;
                dialogEventListener.connect(okButton,
                                            boost::bind(&XULWin::Dialog::endModal, dialog, XULWin::DialogResult_Ok));
                dialog->showModal(mWindow);
            }
        }
        return XULWin::cHandled;
    }


    LRESULT Controller::onStrategySelected(WPARAM wParam, LPARAM lParam)
    {
        if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            std::string strategyName = mStrategiesMenuList->getLabel();
            if (strategyName == "Balanced")
            {
                if (!dynamic_cast<Balanced*>(mEvaluator.get()))
                {
                    mEvaluator.reset(new Balanced);
                    LogInfo("AI is now using the default strategy.");
                }
            }
            else if (strategyName == "Perfectionistic")
            {
                if (!dynamic_cast<Perfectionistic*>(mEvaluator.get()))
                {
                    mEvaluator.reset(new Perfectionistic);
                    LogInfo("AI will now try to make tetrises.");
                }
            }
        }
        // Must return unhandled otherwise the popup menu stays.
        return XULWin::cUnhandled;
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
            ScopedConstAtom<Game> rgame(*mProtectedGame);
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
        ScopedAtom<Game> wgame(*mProtectedGame);
        Game & game = *wgame.get();
        return game.move(inDirection);
    }


    void Controller::drop(TetrisComponent * tetrisComponent)
    {
        if (mComputerPlayer || mBlockMover)
        {
            return;
        }
        ScopedAtom<Game> wgame(*mProtectedGame);
        Game & game = *wgame.get();
        game.drop();
    }


    bool Controller::rotate(TetrisComponent * tetrisComponent)
    {
        if (mComputerPlayer || mBlockMover)
        {
            return false;
        }
        ScopedAtom<Game> wgame(*mProtectedGame);
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
        return *mProtectedGame;
    }


    void Controller::log(const std::string & inMessage)
    {
        if (mLoggingTextBox)
        {
            std::string timestamp = Poco::DateTimeFormatter::format(Poco::DateTime(), "%H:%M:%S ");
            mLoggingTextBox->setValue(MakeString() << mLoggingTextBox->getValue() << "\r\n" << timestamp << inMessage);

            // Scroll to bottom
            int endPos = mLoggingTextBox->getValue().size();
            ::SendMessage(mLoggingTextBox->handle(), EM_SETSEL, (WPARAM)endPos, (LPARAM)endPos);
            ::SendMessage(mLoggingTextBox->handle(), EM_SCROLLCARET, 0, 0);
        }
    }


    int Controller::calculateRemainingTimeMs(Game & game) const
    {
        float numRemainingRows = static_cast<float>(game.currentNode()->state().stats().firstOccupiedRow() - (game.activeBlock().row() + 4));
        float numRowsPerSecond = mTimedGame->currentSpeed();
        float remainingTime = 1000 * numRemainingRows / numRowsPerSecond;
        return static_cast<int>(0.5 + remainingTime);
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

            // If we still have blocks queued, then give the computer 1 second per precalculated block.
            // Note that this may be a misestimation. However, in that case we can still be saved if
            // the refresh() method forces a time-out in time using the Player::setTimeExpired() method.
            int timeLimit = 1000 * blockOffset;
            if (blockOffset > 0)
            {
                BlockTypes futureBlocks_tooMany;
                game.getFutureBlocks(inDepth + blockOffset, futureBlocks_tooMany);

                BlockTypes futureBlocks;
                for (size_t idx = blockOffset; idx != futureBlocks_tooMany.size(); ++idx)
                {
                    futureBlocks.push_back(futureBlocks_tooMany[idx]);
                }
                mComputerPlayer.reset(new Player(endNode, futureBlocks, mEvaluator->clone(), timeLimit));
                mComputerPlayer->start();
                return;
            }


            //
            // We don't have any blocks queued so we have to be very time aware.
            //
            int remainingTimeMs = calculateRemainingTimeMs(game);
            if (remainingTimeMs < 1000)
            {
                if (remainingTimeMs > 500)
                {
                    inDepth = 2;
                }
                else
                {
                    inDepth = 1;
                }
            }

            // We need to start moving before we run out of time!
            timeLimit = remainingTimeMs - 500;

            // We don't even have 500 ms to make our move.
            // This will probably end bad, but we can't always win.
            // Just give it 1 ms. We'll see what it gives.
            if (timeLimit < 0)
            {
                // Just use a dummy value.
                timeLimit = 1;
            }
            
            BlockTypes futureBlocks_tooMany;
            game.getFutureBlocks(inDepth + blockOffset, futureBlocks_tooMany);

            BlockTypes futureBlocks;
            for (size_t idx = blockOffset; idx != futureBlocks_tooMany.size(); ++idx)
            {
                futureBlocks.push_back(futureBlocks_tooMany[idx]);
            }

            mComputerPlayer.reset(new Player(endNode, futureBlocks, mEvaluator->clone(), timeLimit));
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

        ScopedAtom<Game> wgame(*mProtectedGame);
        Game & game = *(wgame.get());
        if (game.isGameOver())
        {
            mStatusTextBox->setValue("Game Over!");
            return;
        }

        
        if (mComputerPlayer)
        {
            // Check if the computer player has finished. If yes, then get the results.
            if (mComputerPlayer->isFinished())
            {
                // Did we manage to think all our moves before the block was dropped?
                GameStateNode * endNode = game.endNode();
                ChildNodePtr resultNode;
                if (mComputerPlayer->result(resultNode))
                {
                    if (resultNode->depth() == endNode->depth() + 1)
                    {
                        endNode->addChild(resultNode);
                    }
                }
                mComputerPlayer.reset();
            }
            // Else check if there is any danger of crashing the current block.
            else
            {
                int remainingTimeMs = calculateRemainingTimeMs(game);

                // Panic! Stop the computer player now or we won't make it!
                if (remainingTimeMs < 1000)
                {
                    mComputerPlayer->setTimeExpired();
                }
            }
        }


        if (mLevelTextBox && mTimedGame)
        {
            mLevelTextBox->setValue(MakeString() << mTimedGame->level());
        }


        if (mScoreTextBox)
        {
            mScoreTextBox->setValue(MakeString() << game.currentNode()->state().stats().score());
        }

                
        if (!mComputerPlayer && mComputerEnabledCheckBox->isChecked())
        {
            int searchDepth = XULWin::String2Int(mSearchDepth->getValue(), 2);
            GameStateNode * endNode = game.endNode();
            if (!endNode->state().isGameOver() &&
                endNode->depth() - game.currentNode()->depth() + searchDepth <=  3 * cMaxSearchDepth)
            {
                startAI(game, searchDepth);
            }
        }


        if (mMovementSpeed && mBlockMover)
        {
            mBlockMover->setSpeed(XULWin::String2Int(mMovementSpeed->getValue(), 1));
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
                    mBlockMover.reset();
                }
            }
            // If no then start the block mover.
            else
            {
                mBlockMover.reset(new BlockMover(*mProtectedGame, XULWin::String2Int(mMovementSpeed->getValue(), 20)));
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
                if (!mBlockMover)
                {
                    status = "Moving in maximum ";
                    int remainingSeconds = static_cast<int>(0.5 + (static_cast<float>(mComputerPlayer->timeRemaining()) / 1000.0));
                    if (remainingSeconds >= 0 && !status.empty())
                    {
                        status += boost::lexical_cast<std::string>(remainingSeconds) + " seconds.";
                    }
                }
                else
                {
                    if (!status.empty())
                    {
                        status += " + ";
                    }
                    status += "Moving.";
                }
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


        for (size_t idx = 0; idx != 4; ++idx)
        {
            const GameState::Stats & stats = game.currentNode()->state().stats();
            if (mLinesTextBoxes[idx])
            {
                mLinesTextBoxes[idx]->setValue(MakeString() << stats.numLines(idx));
            }
        }
        
        if (mGameHeightFactor &&
            mLastBlockHeightFactor &&
            mNumHolesFactor &&
            mNumLinesFactor &&
            mNumSinglesFactor &&
            mNumDoublesFactor &&
            mNumTriplesFactor &&
            mNumTetrisesFactor)
        {
            mEvaluator.reset(new Evaluator(                
                GameHeightFactor(XULWin::String2Int(mGameHeightFactor->getValue(), 0)),
                LastBlockHeightFactor(XULWin::String2Int(mLastBlockHeightFactor->getValue(), 0)),
                NumHolesFactor(XULWin::String2Int(mNumHolesFactor->getValue(), 0)),
                NumSinglesFactor(XULWin::String2Int(mNumSinglesFactor->getValue(), 0)),
                NumDoublesFactor(XULWin::String2Int(mNumDoublesFactor->getValue(), 0)),
                NumTriplesFactor(XULWin::String2Int(mNumTriplesFactor->getValue(), 0)),
                NumTetrisesFactor(XULWin::String2Int(mNumTetrisesFactor->getValue(), 0))));


            if (mGameStateScore)
            {
                mGameStateScore->setValue(XULWin::Int2String(mEvaluator->evaluate(game.currentNode()->state())));
            }
        }


        if (mFPSTextBox)
        {
            mFPSTextBox->setValue(MakeString() << mTetrisComponent->getFPS());
        }
    }


    void Controller::processKey(int inKey)
    {
    }


} // namespace Tetris
