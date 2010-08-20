#include "TetrisElement.h"
#include "Threading.h"
#include "BlockMover.h"
#include "Game.h"
#include "GameCommandQueue.h"
#include "Logger.h"
#include "Player.h"
#include "TimedGame.h"
#include "TetrisComponent.h"
#include "Unicode.h"
#include "XULWin/XULRunner.h"
#include "XULWin/Components.h"
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

    enum
    {
        cAIMaxDepth = 6,        // max depth of the AI
        cAIThinkingTime = 5000  // number of ms the AI is allowed to think
    };

    
    class Controller
    {
    public:
        Controller(HINSTANCE hInstance) :
            mXULRunner(hInstance),
            mBlockCountTextBox(0),
            mLoggingTextBox(0),
            mThreadSafeGame(),
            mTimedGame(),
            mRefreshTimer(),
            mBlockMover(),
            mAIThread(),
            mTimedNodePopulator()
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
            // Activate the stats updater.
            //
            mRefreshTimer.reset(new XULWin::WinAPI::Timer);
            mRefreshTimer->start(boost::bind(&Controller::updateStats, this), 100);


            //
            // Show the Window.
            //
            wnd->showModal(XULWin::WindowPos_CenterInScreen);
        }


        ~Controller()
        {
            mTimedGame.reset();

        }


        void joinAllThreads()
        {
            if (mAIThread)
            {
                mAIThread->join();
            }
        }


        ThreadSafeGame & threadSafeGame()
        {
            return *mThreadSafeGame;
        }


        void log(const std::string & inMessage)
        {
            if (mLoggingTextBox)
            {
                mLoggingTextBox->setValue(inMessage + "\r\n" + mLoggingTextBox->getValue());
            }
        }


        void startAI()
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


        void updateStats()
        {
            if (mBlockCountTextBox)
            {
                size_t currentBlockIndex = ReadOnlyGame(*mThreadSafeGame)->currentBlockIndex();
                mBlockCountTextBox->setValue(MakeString() << currentBlockIndex);
            }
        }

            
        void precalculate()
        {
            // Thead entry point
            try
            {
                // Start the block mover
                mBlockMover.reset(new BlockMover(*mThreadSafeGame));

                // Infinite AI
                while (true)
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


        void calculateOptimalPath(std::auto_ptr<GameStateNode> inClonedGameState, const BlockTypes & inBlockTypes)
        {
            //
            // Start the number crunching. This will take a while to complete (max cAIThinkingTime).
            //
            int currentGameDepth = inClonedGameState->depth();
            mTimedNodePopulator.reset(new TimedNodePopulator(inClonedGameState, inBlockTypes, cAIThinkingTime));
            mTimedNodePopulator->start(); // Wait for results... (limited by cAIThinkingTime)


            //
            // Eliminate the inferior branches. The tree will be reduced to a path of nodes defining
            // the best game strategy.
            //
            ChildNodePtr bestLastChild = mTimedNodePopulator->getBestChild();
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
            for (ChildNodes::iterator it = mTimedNodePopulator->node()->children().begin(); it != mTimedNodePopulator->node()->children().end(); ++it)
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
            
            // Critical section: insert the first child in the current node's child list.
            {
                WritableGame game(*mThreadSafeGame);
                GameStateNode & currentNode = *game->currentNode();
                if (currentNode.children().empty())
                {
                    currentNode.children().insert(firstChild);
                }
            }
        }

        void processKey(int inKey)
        {
            if (inKey == VK_DELETE)
            {
                startAI();
            }
        }

    private:
        XULWin::XULRunner mXULRunner;
        XULWin::TextBox * mBlockCountTextBox;
        XULWin::TextBox * mLoggingTextBox;
        boost::scoped_ptr<ThreadSafeGame> mThreadSafeGame;
        boost::scoped_ptr<Tetris::TimedGame> mTimedGame;
        boost::scoped_ptr<XULWin::WinAPI::Timer> mRefreshTimer;
        boost::scoped_ptr<BlockMover> mBlockMover;
        boost::scoped_ptr<boost::thread> mAIThread;
        boost::scoped_ptr<TimedNodePopulator> mTimedNodePopulator;
    };


} // namespace Tetris


int StartProgram(HINSTANCE hInstance)
{
    XULWin::Initializer initializer(hInstance);
    XULWin::WinAPI::CommonControlsInitializer ccInit;
    XULWin::ElementFactory::Instance().registerElement<Tetris::TetrisElement>();

    // Create the Controller object
    Tetris::Controller controller(hInstance);
    controller.joinAllThreads();
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
