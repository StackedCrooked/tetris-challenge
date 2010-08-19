#include "TetrisElement.h"
#include "Threading.h"
#include "Game.h"
#include "GameCommandQueue.h"
#include "Logger.h"
#include "Player.h"
#include "TimedGame.h"
#include "TetrisComponent.h"
#include "Unicode.h"
#include "XULWin/XULRunner.h"
#include "XULWin/Decorator.h"
#include "XULWin/ElementFactory.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/EventListener.h"
#include "XULWin/Element.h"
#include "XULWin/Component.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/EventListener.h"
#include "XULWin/Initializer.h"
#include "XULWin/Window.h"
#include "XULWin/WinUtils.h"
#include "XULWin/Unicode.h"
#include <boost/scoped_ptr.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <windows.h>
#include <shellapi.h>


namespace Tetris
{

    enum {
        cAIMaxDepth = 10,       // max depth of the AI
        cAIThinkingTime = 5000  // number of ms the AI is allowed to think
    };

    boost::scoped_ptr<Tetris::GameCommandQueue> gCommander;
    boost::scoped_ptr<boost::thread> gThread;


    class BlockMover
    {
    public:
        BlockMover(ThreadSafeGame & inGame,
                   boost::function<void()> inFinishedCallback) :
            mGame(inGame),
            mFinished(false),
            mFinishedCallback(inFinishedCallback)
        {
            mTimer.start(boost::bind(&BlockMover::onTimer, this), 10);
        }       

        bool isFinished()
        {
            return mFinished;
        }

    private:
        void onTimer()
        {
            try
            {
                move();
            }
            catch (const std::exception & inException)
            {
                LogError(inException.what());

                // Critical section
                {
                    WritableGame wg(gCommander->threadSafeGame());
                    Game & game = *wg.get();
                    
                    // Try to cheat our way out of it
                    if (game.navigateNodeDown())
                    {
                        LogWarning("I can't move the block to where I planned to so I'm cheating now.");
                    }
                    else
                    {
                        LogError("Application state has become corrupted for an unknown reason. Clearing all AI data.");
                        mTimer.stop();
                        game.currentNode()->children().clear();
                        mFinished = true;
                    }
                }
            }
        }

        void move()
        {

            // Critical section
            while (true)
            {
                WritableGame game(mGame);
                ChildNodes & children = game->currentNode()->children();
                if (children.empty())
                {
                    mFinished = true;
                    break;
                }

                Block & block = game->activeBlock();
                const Block & targetBlock = (*children.begin())->state().originalBlock();
                if (block.rotation() != targetBlock.rotation())
                {
                    if (!game->rotate())
                    {
                        throw std::runtime_error(MakeString() << "Rotation failed.");
                    }
                }
                else if (block.column() < targetBlock.column())
                {
                    if (!game->move(Direction_Right))
                    {
                        throw std::runtime_error(MakeString() << "Move to right failed. Current column: " << block.column()
                                                              << ", target column: " << targetBlock.column() << ".");
                    }
                }
                else if (block.column() > targetBlock.column())
                {
                    if (!game->move(Direction_Left))
                    {
                        throw std::runtime_error(MakeString() << "Move to left failed. "
                                                              << "Current column: " << block.column() << ", "
                                                              << "target column: " << targetBlock.column() << ".");
                    }
                }
                else
                {
                    GameState & gameState = game->currentNode()->state();
                    if (gameState.checkPositionValid(block, block.row() + 1, block.column()))
                    {
                        game->move(Direction_Down);
                    }
                    else if (!game->navigateNodeDown())
                    {
                        throw std::runtime_error("Failed to navigate the game state one node down.");
                    }
                }
                break;
            }

            if (mFinished)
            {
                mTimer.stop();
                if (mFinishedCallback)
                {
                    mFinishedCallback();
                }
            }
        }        
        
        ThreadSafeGame & mGame;
        XULWin::WinAPI::Timer mTimer; // Use non-threaded timer where we can.
        bool mFinished;
        boost::function<void()> mFinishedCallback;
    };

    boost::scoped_ptr<BlockMover> gBlockMover;


    void StartAI();


    void MoveNextQueuedBlock()
    {
        // Move to the next precalculated position.
        // Note: AI should use this time for calculating next moves.
        if (!ReadOnlyGame(gCommander->threadSafeGame())->currentNode()->children().empty())
        {
            gBlockMover.reset(new BlockMover(gCommander->threadSafeGame(),
                                             boost::bind(MoveNextQueuedBlock)));
        }
        else
        {
            // Play AI endlessly
            StartAI();
        }

    }


    void StartAI()
    {
        if (gBlockMover && !gBlockMover->isFinished())
        {
            // Busy moving blocks.
            LogWarning("Can't start AI while moving blocks.");
            return;
        }

        // Critical section
        {
            ReadOnlyGame game(gCommander->threadSafeGame());
            ChildNodes children = game->currentNode()->children();
            if (!children.empty())
            {
                assert(children.size() == 1);

                // There are still some queued blocks that need to be moved by the blockmover.
                LogWarning("AI not started is still moving the blocks.");
                return;
            }
        }


        //
        // No more queued moves, so let's create some...
        //

        BlockTypes blockTypes;
        std::auto_ptr<GameStateNode> clonedGameState;

        // Critical section
        {
            WritableGame game(gCommander->threadSafeGame());
            game->getFutureBlocks(cAIMaxDepth, blockTypes);
            clonedGameState = game->currentNode()->clone();
        }

        int currentGameDepth = clonedGameState->depth();
        TimedNodePopulator populator(clonedGameState, blockTypes, cAIThinkingTime);
        populator.start();                
        ChildNodePtr bestLastChild = populator.getBestChild();
        GameStateNode * bestFirstChild = bestLastChild.get();
        int realDepth = 0;
        while (bestFirstChild->depth() > currentGameDepth + 1)
        {
            bestFirstChild = bestFirstChild->parent();
            realDepth++;
        }
        LogInfo(MakeString() << "AI searched " << (bestLastChild->depth() - bestFirstChild->depth()) << " nodes deep.");
        LogInfo(MakeString() << "Verified depth: " << realDepth << ".");
        DestroyInferiorChildren(bestFirstChild, bestLastChild.get());
        
        ChildNodePtr firstChild;
        for (ChildNodes::iterator it = populator.node()->children().begin(); it != populator.node()->children().end(); ++it)
        {
            ChildNodePtr child = *it;
            if (child.get() == bestFirstChild)
            {
                firstChild = child;
                break;
            }
        }

        assert(firstChild);

        // Critical section
        {
            WritableGame game(gCommander->threadSafeGame());
            assert(game->currentNode()->children().empty()); // FOR NOW!
            game->currentNode()->children().insert(firstChild);
        }

        const Block & targetBlock = firstChild->state().originalBlock();
        assert(firstChild->state().checkPositionValid(targetBlock, targetBlock.rotation(), targetBlock.column()));
        gBlockMover.reset(
            new BlockMover(gCommander->threadSafeGame(),
                           boost::bind(MoveNextQueuedBlock)));
    }


    void ProcessKey(int inKey)
    {
        if (inKey == VK_DELETE)
        {
            StartAI();
        }
    }

}


using namespace Tetris;


void AppendLog(XULWin::Element * inTextBoxElement, const std::string & inMessage)
{
    inTextBoxElement->setAttribute("value", inTextBoxElement->getAttribute("value") + inMessage + "\r\n");
}


void UpdateStats(XULWin::Element * inBlocksTextBox)
{
    ReadOnlyGame game(gCommander->threadSafeGame());
    inBlocksTextBox->setAttribute("value", MakeString() << game->currentBlockIndex());
}


int StartProgram(HINSTANCE hInstance)
{
    XULWin::Initializer initializer(hInstance);
    XULWin::WinAPI::CommonControlsInitializer ccInit;
    XULWin::ElementFactory::Instance().registerElement<Tetris::TetrisElement>();

    XULWin::ErrorCatcher errorCatcher;
    errorCatcher.disableLogging(true);

    XULWin::XULRunner runner(hInstance);
    XULWin::ElementPtr rootElement = runner.loadXULFromFile("XULWinTetrisTest.xul");
    if (!rootElement)
    {
        XULWin::ReportError("Failed to load the root element");
        return 1;
    }

    XULWin::Window * wnd = rootElement->component()->downcast<XULWin::Window>();
    if (!wnd)
    {
        XULWin::ReportError("Root element is not of type winodw.");
        return 1;
    }

    Tetris::TetrisComponent * tetrisComponent(0);
    std::vector<Tetris::TetrisElement *> tetrisElements;
    rootElement->getElementsByType<Tetris::TetrisElement>(tetrisElements);
    for (size_t idx = 0; idx != tetrisElements.size(); ++idx)
    {
        tetrisComponent = tetrisElements[idx]->component()->downcast<Tetris::TetrisComponent>();
        break;
    }

    if (!tetrisComponent)
    {
        return 0;
    }

    // Init the logging
    if (XULWin::Element * loggingElement = rootElement->getElementById("logging-textbox"))
    {
        Tetris::Logger::Instance().setHandler(boost::bind(AppendLog, loggingElement, _1));
    }

    // Add the timer
    Tetris::TimedGame timedGame(tetrisComponent->getThreadSafeGame());
    timedGame.start();


    // Init the command queue
    gCommander.reset(new Tetris::GameCommandQueue(tetrisComponent->getThreadSafeGame()));

    // Register the keyboard listener
    tetrisComponent->OnKeyboardPressed.connect(boost::bind(Tetris::ProcessKey, _1));

    LogInfo("Press the DELETE button to let the computer play");

    XULWin::WinAPI::Timer timer;
    timer.start(boost::bind(UpdateStats, rootElement->getElementById("blocksTextBox")), 100);

    wnd->showModal(XULWin::WindowPos_CenterInScreen);
    gCommander.reset();
    gBlockMover.reset();
    return 0;
}


INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        return StartProgram(hInstance);
    }
    catch (const std::exception & inError)
    {
        ::MessageBox(0, XULWin::ToUTF16(inError.what()).c_str(), L"XULWin Tetris Component", MB_OK);
    }
    catch (...)
    {
        ::MessageBox(0, TEXT("Program is terminated due to unhandled and unknown exception."), L"XULWin Tetris Component", MB_OK);
    }
    return 0;
}
