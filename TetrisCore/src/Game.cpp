#include "Game.h"
#include "Block.h"
#include "ErrorHandling.h"
#include "GameState.h"


namespace Tetris
{

    static std::auto_ptr<Block> CenterBlock(std::auto_ptr<Block> inNewBlock, size_t inNumColumns)
    {
        size_t column = static_cast<int>(0.5 + (static_cast<float>(inNumColumns - inNewBlock->grid().numColumns())/2));
        inNewBlock->setColumn(column);
        return inNewBlock;
    }


    Game::Game(int inNumRows, int inNumColumns) :
        mRootNode(std::auto_ptr<GameState>(new GameState(inNumRows, inNumColumns))),
        mBlockFactory(cBlockTypeCount),
        mBlock(CenterBlock(mBlockFactory.getNext(), inNumColumns))
    {
        mCurrentNode = &mRootNode;
    }


    bool Game::isGameOver() const
    {
        return mCurrentNode->state().isGameOver();
    }


    const Block & Game::activeBlock() const
    {
        CheckPrecondition(mBlock.get() != 0, "Tried to dereference null pointer.");
        return *mBlock;
    }


    Block & Game::activeBlock()
    {
        CheckPrecondition(mBlock.get() != 0, "Tried to dereference null pointer.");
        return *mBlock;
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
        size_t newRow = mBlock->row() + GetRowDelta(inDirection);
        size_t newCol = mBlock->column() + GetColumnDelta(inDirection);
        bool isMoveAllowed = gameState.checkPositionValid(*mBlock, newRow, newCol);
        if (isMoveAllowed)
        {
            mBlock->setRow(newRow);
            mBlock->setColumn(newCol);
        }
        return isMoveAllowed;
    }


    void Game::rotate()
    {
        mBlock->rotate();
    }


    void Game::moveDown()
    {
        const GameState & gameState = mCurrentNode->state();

        bool canMoveDown = gameState.checkPositionValid(*mBlock,
                                                        mBlock->row() + 1,
                                                        mBlock->column());
        if (canMoveDown)
        {
            mBlock->moveDown();
            return;
        }

        // If we can't move a new block down then the game is over.
        bool gameOver = (mBlock->row() == 0);
        ChildPtr child(new GameStateNode(mCurrentNode->state().commit(mBlock, gameOver)));
        mCurrentNode->children().insert(child);
        mCurrentNode = child.get();
        
        // Currently mBlock is a null pointer (due to the copy semantics of std::auto_ptr).
        // Therefore we always (also in the case of game-over) get a new one from the factory.
        mBlock = CenterBlock(mBlockFactory.getNext(), gameState.grid().numColumns());
    }


} // namespace Tetris

