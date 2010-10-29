#include "Tetris/Config.h"
#include "Tetris/GameState.h"
#include "Tetris/GameQualityEvaluator.h"
#include "Tetris/Block.h"
#include "Tetris/BlockType.h"
#include "Tetris/Grid.h"
#include "Tetris/Assert.h"
#include <algorithm>


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
        GameStateImpl(size_t inNumRows, size_t inNumColumns) :
            mGrid(inNumRows, inNumColumns, BlockType_Nil),
            mStats(),
            mIsGameOver(false),
            mOriginalBlock(BlockType_L, Rotation(0), Row(0), Column(0)),
            mQuality()
        {
            mStats.mFirstOccupiedRow = inNumRows;
        }

        void solidifyBlock(const Block & inBlock)
        {
            const Grid & grid = inBlock.grid();
            for (size_t r = 0; r != grid.numRows(); ++r)
            {
                for (size_t c = 0; c != grid.numColumns(); ++c)
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

        
        void clearLines()
        {
            size_t numLines = 0;
            
            std::vector<char> linesVector(mGrid.numRows(), 0);
            char * lines = &linesVector[0];
            //std::vector<char> lines(mGrid.numRows(), 0);

            size_t endRow = std::min<size_t>(mGrid.numRows(), mOriginalBlock.row() + mOriginalBlock.grid().numRows());
            for (size_t rowIndex = mOriginalBlock.row(); rowIndex < endRow; ++rowIndex)
            {
                lines[rowIndex] = 1;
                for (size_t ci = 0; ci != mGrid.numColumns(); ++ci)
                {
                    if (mGrid.get(rowIndex, ci) == 0)
                    {
                        lines[rowIndex] = 0;
                        break;
                    }
                }

                if (lines[rowIndex])
                {
                    numLines++;
                }
            }

            if (numLines == 0)
            {
                return;
            }

            Assert(mStats.mFirstOccupiedRow + numLines <= mGrid.numRows());
            mStats.mFirstOccupiedRow += numLines;

            // Get newGrid
            Grid newGrid(mGrid.numRows(), mGrid.numColumns(), BlockType_Nil);
            int newRowIdx = numLines;
            for (size_t r = 0; r != mGrid.numRows(); ++r)
            {
                if (!lines[r])
                {
                    for (size_t c = 0; c != mGrid.numColumns(); ++c)
                    {
                        newGrid.set(newRowIdx, c, mGrid.get(r, c));
                    }
                    newRowIdx++;
                }
            }

            mGrid = newGrid;

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

        Grid mGrid;
        Stats mStats;
        bool mIsGameOver;
        Block mOriginalBlock;
        mutable Quality mQuality;
    };


    GameState::GameState(size_t inNumRows, size_t inNumColumns) :
        mImpl(new GameStateImpl(inNumRows, inNumColumns))
    {
    }


    GameState::GameState(const GameState & rhs) :
        mImpl(new GameStateImpl(*rhs.mImpl))
    {
        
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


    int GameState::quality() const
    {
        Assert(mImpl->mQuality.isInitialized());
        return mImpl->mQuality.score();
    }


    bool GameState::checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
    {
        if (inRowIdx >= mImpl->mGrid.numRows() || inColIdx >= mImpl->mGrid.numColumns())
        {
            return false;
        }

        for (size_t r = 0; r != inBlock.grid().numRows(); ++r)
        {
            for (size_t c = 0; c != inBlock.grid().numColumns(); ++c)
            {

                if (inBlock.grid().get(r, c) != 0)
                {
                    size_t rowIdx = inRowIdx + r;
                    if (rowIdx >= mImpl->mGrid.numRows())
                    {
                        return false;
                    }

                    size_t colIdx = inColIdx + c;
                    if (colIdx >= mImpl->mGrid.numColumns())
                    {
                        return false;
                    }

                    if (mImpl->mGrid.get(rowIdx, colIdx) != BlockType_Nil)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
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

} // namespace Tetris

