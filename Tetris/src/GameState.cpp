#include "Tetris/Config.h"
#include "Tetris/GameState.h"
#include "Tetris/Evaluator.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"
#include "Tetris/Assert.h"
#include <algorithm>
#include <vector>


namespace Tetris
{
    
    class Quality
    {
    public:
        Quality() :
            mIsInitialized(false),
            mScore(0),
            mNumHoles(0)
        {
        }

        bool isInitialized() const
        {
            return mIsInitialized;
        }

        void setInitialized(bool inIsInitialized)
        {
            mIsInitialized = inIsInitialized;
        }

        int score() const
        {
            return mScore;
        }

        void setScore(int inScore)
        {
            mScore = inScore;
        }

        int numHoles() const
        {
            return mNumHoles;
        }

        void setNumHoles(int inNumHoles)
        {
            mNumHoles = inNumHoles;
        }

        void reset()
        {
            mScore = 0;
            mNumHoles = 0;
        }

    private:
        bool mIsInitialized;
        int mScore;
        int mNumHoles;
    };


    Stats::Stats() :
        mNumLines(0),
        mNumSingles(0),
        mNumDoubles(0),
        mNumTriples(0),
        mNumTetrises(0),
        mFirstOccupiedRow(0)
    {
    }

    int Stats::score() const
    {
        return 40 * mNumSingles + 100 * mNumDoubles + 300 * mNumTriples + 1200 * mNumTetrises;
    }

    int Stats::numLines() const
    {
        return mNumLines;
    }

    int Stats::numSingles() const
    {
        return mNumSingles;
    }

    int Stats::numDoubles() const
    {
        return mNumDoubles;
    }

    int Stats::numTriples() const
    {
        return mNumTriples;
    }

    int Stats::numTetrises() const
    {
        return mNumTetrises;
    }

    int Stats::firstOccupiedRow() const
    {
        return mFirstOccupiedRow;
    }

    // Use zero based index!
    int Stats::numLines(size_t idx) const
    {
        switch (idx)
        {
            case 0: return mNumSingles;
            case 1: return mNumDoubles;
            case 2: return mNumTriples;
            case 3: return mNumTetrises;
            default: throw std::invalid_argument("Invalid number of lines scored requested.");
        }
        return 0; // compiler happy
    }


    class GameStateImpl
    {
    public:
        GameStateImpl(size_t inNumRows, size_t inNumColumns);

        void solidifyBlock(const Block & inBlock);
        
        void clearLines();

        void forceUpdateStats();

        bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

    private:
        friend class GameState;
        Grid mGrid;
        Stats mStats;
        bool mIsGameOver;
        Block mOriginalBlock;
        mutable Quality mQuality;
    };


    GameStateImpl::GameStateImpl(size_t inNumRows, size_t inNumColumns) :
        mGrid(inNumRows, inNumColumns, BlockType_Nil),
        mStats(),
        mIsGameOver(false),
        mOriginalBlock(BlockType_L, Rotation(0), Row(0), Column(0)),
        mQuality()
    {
        mStats.mFirstOccupiedRow = inNumRows;
    }


    bool GameStateImpl::checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
    {
        const Grid & blockGrid(inBlock.grid());
        if (inColIdx < blockGrid.columnCount() &&
            (inRowIdx + blockGrid.rowCount()) < static_cast<size_t>(mStats.firstOccupiedRow()))
        {
            return true;
        }

        if (inRowIdx >= mGrid.rowCount())
        {
            return false;
        }

        for (size_t r = 0; r != blockGrid.rowCount(); ++r)
        {
            size_t rowIdx = inRowIdx + r;
            if (rowIdx >= mGrid.rowCount())
            {
                return false;
            }

            for (size_t c = 0; c != blockGrid.columnCount(); ++c)
            {
                size_t colIdx = inColIdx + c;
                if (colIdx >= mGrid.columnCount())
                {
                    return false;
                }

                if (blockGrid.get(r, c) && mGrid.get(rowIdx, colIdx))
                {
                    return false;
                }
            }
        }
        return true;
    }


    void GameStateImpl::forceUpdateStats()
    {
        for (size_t r = 0; r != mGrid.rowCount(); ++r)
        {
            for (size_t c = 0; c != mGrid.columnCount(); ++c)
            {
                if (mGrid.get(r, c) != BlockType_Nil)
                {
                    mStats.mFirstOccupiedRow = r;
                    return;
                }
            }
        }
    }


    void GameStateImpl::solidifyBlock(const Block & inBlock)
    {
        const Grid & grid = inBlock.grid();
        for (size_t r = 0; r != grid.rowCount(); ++r)
        {
            for (size_t c = 0; c != grid.columnCount(); ++c)
            {
                if (grid.get(r, c) != BlockType_Nil)
                {
                    int gridRow = inBlock.row() + r;
                    int gridCol = inBlock.column() + c;
                    mGrid.set(gridRow, gridCol, inBlock.type());
                    if (gridRow < mStats.mFirstOccupiedRow)
                    {
                        mStats.mFirstOccupiedRow = gridRow;
                    }
                }
            }
        }
    }

    
    void GameStateImpl::clearLines()
    {
        int numLines = 0;
        int r = mOriginalBlock.row() + mOriginalBlock.rowCount() - 1;
        for (; r >= mStats.firstOccupiedRow(); --r)
        {
            int c = 0;
            bool line = true;
            for (; c < mGrid.columnCount(); ++c)
            {
                BlockType value = mGrid.get(r, c);
                if (numLines > 0)
                {
                    mGrid.set(r + numLines, c, value);
                }

                if (!value)
                {
                    line = false;
                    if (numLines == 0)
                    {
                        break;
                    }
                }
            }
            if (line)
            {
                numLines++;
            }
        }

        if (numLines > 0)
        {
            BlockType * gridBegin = const_cast<BlockType*>(&(mGrid.get(mStats.firstOccupiedRow(), 0)));
            memset(&gridBegin[0], 0, numLines * mGrid.columnCount());
        }
		
        Assert(mStats.mFirstOccupiedRow + numLines <= mGrid.rowCount());
        mStats.mFirstOccupiedRow += numLines;
        mStats.mNumLines += numLines;

        switch (numLines)
        {
            case 0:
            {
                // Do nothing.
                break;
            }
            case 1:
            {
                mStats.mNumSingles++;
                break;
            }
            case 2:
            {
                mStats.mNumDoubles++;
                break;
            }
            case 3:
            {
                mStats.mNumTriples++;
                break;
            }
            case 4:
            {
                mStats.mNumTetrises++;
                break;
            }
            default:
            {
                throw std::logic_error("Invalid number of lines scored! ...?");
            }
        }
    }


    GameState::GameState(size_t inNumRows, size_t inNumColumns) :
        mImpl(new GameStateImpl(inNumRows, inNumColumns))
    {
    }


    GameState::GameState(const GameState & rhs) :
        mImpl(new GameStateImpl(*rhs.mImpl))
    {
        
    }


    GameState::~GameState()
    {
        delete mImpl;
        mImpl = 0;
    }


    bool GameState::isGameOver() const
    {
        return mImpl->mIsGameOver;
    }


    const Grid & GameState::grid() const
    {
        return mImpl->mGrid;
    }


    Grid & GameState::grid()
    {
        return mImpl->mGrid;
    }


    int GameState::quality(const Evaluator & inEvaluator) const
    {
        if (!mImpl->mQuality.isInitialized())
        {
            mImpl->mQuality.reset();
            mImpl->mQuality.setScore(inEvaluator.evaluate(*this));
            mImpl->mQuality.setInitialized(true);
        }
        return mImpl->mQuality.score();
    }


    bool GameState::checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
    {
        return mImpl->checkPositionValid(inBlock, inRowIdx, inColIdx);
    }


    const Block & GameState::originalBlock() const
    {
        return mImpl->mOriginalBlock;
    }


    std::auto_ptr<GameState> GameState::commit(const Block & inBlock, GameOver inGameOver) const
    {
        std::auto_ptr<GameState> result(new GameState(*this));
        result->mImpl->mIsGameOver = inGameOver.get();
        result->mImpl->mQuality.setInitialized(false);
        if (!inGameOver.get())
        {
            result->mImpl->solidifyBlock(inBlock);
        }
        result->mImpl->mOriginalBlock = inBlock;
        result->mImpl->clearLines();
        return result;
    }


    const Stats & GameState::stats() const
    {
        return mImpl->mStats;
    }


    void GameState::forceUpdateStats()
    {
        mImpl->forceUpdateStats();
    }

} // namespace Tetris

