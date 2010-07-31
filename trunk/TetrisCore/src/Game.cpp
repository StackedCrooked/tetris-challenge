#include "Game.h"
#include "Block.h"
#include "ErrorHandling.h"
#include "GameState.h"


namespace Tetris
{

    static std::auto_ptr<ActiveBlock> CreateCenteredActiveBlock(std::auto_ptr<Block> inNewBlock, size_t inNumColumns)
    {
        size_t middleX = static_cast<int>(0.5 + (static_cast<float>(inNumColumns - inNewBlock->grid().numColumns())/2));
        return std::auto_ptr<ActiveBlock>(new ActiveBlock(inNewBlock, 0, middleX));
    }


    Game::Game(int inNumRows, int inNumColumns) :
        mRootNode(std::auto_ptr<GameState>(new GameState(inNumRows, inNumColumns))),
        mBlockFactory(cBlockTypeCount),
        mActiveBlock(CreateCenteredActiveBlock(mBlockFactory.getNext(), inNumColumns))
    {
        mCurrentNode = &mRootNode;
    }


    bool Game::isGameOver() const
    {
        return mCurrentNode->state().isGameOver();
    }


    const ActiveBlock & Game::activeBlock() const
    {
        CheckPrecondition(mActiveBlock.get() != 0, "Tried to dereference null pointer.");
        return *mActiveBlock;
    }


    ActiveBlock & Game::activeBlock()
    {
        CheckPrecondition(mActiveBlock.get() != 0, "Tried to dereference null pointer.");
        return *mActiveBlock;
    }


    GameStateNode & Game::currentNode()
    {
        return *mCurrentNode;
    }


    const GameStateNode & Game::currentNode() const
    {
        return *mCurrentNode;
    }


    static int GetRowDelta(Direction inDirection)
    {
        switch (inDirection)
        {
            case Direction_Up:
            {
                return -1;
            }
            case Direction_Down:
            {
                return 1;
            }
            default:
            {
                return 0;
            }
        }
    }


    static int GetColumnDelta(Direction inDirection)
    {
        switch (inDirection)
        {
            case Direction_Left:
            {
                return -1;
            }
            case Direction_Right:
            {
                return 1;
            }
            default:
            {
                return 0;
            }
        }
    }


    bool Game::move(Direction inDirection)
    {
        const GameState & gameState = mCurrentNode->state();
        size_t newRow = mActiveBlock->row() + GetRowDelta(inDirection);
        size_t newCol = mActiveBlock->column() + GetColumnDelta(inDirection);
        bool isMoveAllowed = gameState.checkPositionValid(mActiveBlock->block(), newRow, newCol);
        if (isMoveAllowed)
        {
            mActiveBlock->setRow(newRow);
            mActiveBlock->setColumn(newCol);
        }
        return isMoveAllowed;
    }


    void Game::rotate()
    {
        const Block & block = mActiveBlock->block();
        mActiveBlock.reset(new ActiveBlock(std::auto_ptr<Block>(new Block(block.type(), (block.rotation() + 1)%4)), mActiveBlock->row(), mActiveBlock->column()));
    }


    void Game::moveDown()
    {
        const GameState & gameState = mCurrentNode->state();

        bool canMoveDown = gameState.checkPositionValid(mActiveBlock->block(),
                                                        mActiveBlock->row() + 1,
                                                        mActiveBlock->column());
        if (canMoveDown)
        {
            mActiveBlock->moveDown();
            return;
        }

        // If we can't move a new block down then the game is over.
        bool gameOver = (mActiveBlock->row() == 0);
        ChildPtr child(new GameStateNode(mCurrentNode->state().commit(mActiveBlock, gameOver)));
        mCurrentNode->children().insert(child);
        mCurrentNode = child.get();
        
        // Currently mActiveBlock is a null pointer (due to the copy semantics of std::auto_ptr).
        // Therefore we always (also in the case of game-over) get a new one from the factory.
        std::auto_ptr<Block> newBlock(mBlockFactory.getNext());
        size_t middleX = static_cast<int>(0.5 + (static_cast<float>(gameState.grid().numColumns() - newBlock->grid().numColumns())/2));
        mActiveBlock.reset(new ActiveBlock(newBlock, 0, middleX));
    }


} // namespace Tetris

