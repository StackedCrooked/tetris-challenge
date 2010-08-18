#include "GameState.h"
#include "BlockType.h"
#include <algorithm>


namespace Tetris
{

    GameState::GameState(int inNumRows, int inNumColumns) :
        mGrid(inNumRows, inNumColumns, BlockType_Nil),
        mStats(),
        mIsGameOver(false),
        mOriginalBlock(BlockType_L, Rotation(0), Row(0), Column(0)),
        mQuality()
    {
    }


    bool GameState::isGameOver() const
    {
        return mIsGameOver;
    }


    const Grid & GameState::grid() const
    {
        return mGrid;
    }


    Grid & GameState::grid()
    {
        return mGrid;
    }


    int GameState::quality() const
    {
        if (!mQuality.isInitialized())
        {
            mQuality.reset();

            int result = 0;

            bool foundTop = false;
            size_t top = mGrid.numRows();

            size_t numOccupiedUnderTop = 0;

            for (size_t rowIdx = 0; rowIdx != mGrid.numRows(); ++rowIdx)
            {
                for (size_t colIdx = 0; colIdx != mGrid.numColumns(); ++colIdx)
                {
                    const int & value = mGrid.get(rowIdx, colIdx);
                    if (value != BlockType_Nil)
                    {
                        if (!foundTop)
                        {
                            top = rowIdx;
                            foundTop = true;
                        }

                        if (foundTop)
                        {
                            numOccupiedUnderTop++;
                        }
                    }
                    else
                    {
                        // check for holes
                        if (foundTop && rowIdx > 0)
                        {
                            if (mGrid.get(rowIdx - 1, colIdx) != BlockType_Nil)
                            {
                                mQuality.setNumHoles(mQuality.numHoles() + 1);
                            }
                        }
                    }
                }
            }
            size_t height = mGrid.numRows() - top;
            size_t lastBlockHeight = mGrid.numRows() - mOriginalBlock.row();

            result -= 5 * mQuality.numHoles();
            result -= 1 * height;
            result -= 2 * lastBlockHeight;

            result -= 8 * mStats.numSingles();
            result -= 4 * mStats.numDoubles();
            result -= 2 * mStats.numTriples();
            result += 8 * mStats.numTetrises();

            mQuality.setScore(result);
            mQuality.setInitialized(true);
        }
        return mQuality.score();
    }


    bool GameState::checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const
    {
        if (inRowIdx >= mGrid.numRows() || inColIdx >= mGrid.numColumns())
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
                    if (rowIdx >= mGrid.numRows())
                    {
                        return false;
                    }

                    size_t colIdx = inColIdx + c;
                    if (colIdx >= mGrid.numColumns())
                    {
                        return false;
                    }

                    if (mGrid.get(rowIdx, colIdx) != BlockType_Nil)
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
        return mOriginalBlock;
    }


    std::auto_ptr<GameState> GameState::commit(const Block & inBlock, GameOver inGameOver) const
    {
        std::auto_ptr<GameState> result(new GameState(*this));
        result->mIsGameOver = inGameOver.get();
        result->mQuality.setInitialized(false);
        if (!inGameOver.get())
        {
            result->solidifyBlock(inBlock);
        }
        result->mOriginalBlock = inBlock;
        result->clearLines();
        return result;
    }


    const GameState::Stats & GameState::stats() const
    {
        return mStats;
    }


    void GameState::solidifyBlock(const Block & inBlock)
    {
        const Grid & grid = inBlock.grid();
        for (size_t r = 0; r != grid.numRows(); ++r)
        {
            for (size_t c = 0; c != grid.numColumns(); ++c)
            {
                if (grid.get(r, c) != BlockType_Nil)
                {
                    mGrid.set(inBlock.row() + r, inBlock.column() + c, inBlock.type());
                }
            }
        }
    }

    
    void GameState::clearLines()
    {
        size_t numLines = 0;
        std::vector<bool> lines(mGrid.numRows(), false);
        
        // Get numLines and lines
        {
            for (size_t rowIndex = mOriginalBlock.row();
                 rowIndex < std::min<size_t>(mGrid.numRows(),
                                             mOriginalBlock.row() + mOriginalBlock.grid().numRows());
                 ++rowIndex)
            {  
                lines[rowIndex] = true;
                for (size_t ci = 0; ci != mGrid.numColumns(); ++ci)
                {
                    if (mGrid.get(rowIndex, ci) == 0)
                    {
                        lines[rowIndex] = false;
                        break;
                    }
                }

                if (lines[rowIndex])
                {
                    numLines++;
                }
            }
        }

        if (numLines == 0)
        {
            return;
        }

        
        BlockTypes newData(mGrid.data().size(), BlockType_Nil);
        
        // Get newData
        {
            int newRowIdx = numLines;
            for (size_t r = 0; r != mGrid.numRows(); ++r)
            {
				if (!lines[r])
				{
					for (size_t c = 0; c != mGrid.numColumns(); ++c)
					{
						newData[newRowIdx * mGrid.numColumns() + c] = mGrid.get(r, c);
					}					
                    newRowIdx++;
				}
            }
        }
        
        mGrid.data().swap(newData);
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


} // namespace Tetris

