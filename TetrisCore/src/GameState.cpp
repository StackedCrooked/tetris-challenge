#include "GameState.h"
#include "Block.h"
#include <algorithm>


namespace Tetris
{

    size_t Half(size_t inValue)
    {
        return (int)((inValue / 2.0) + 0.5);
    }


    GameState::GameState(int inNumRows, int inNumColumns) :
        mGrid(inNumRows, inNumColumns, BlockType_Nil),
        mStats(),
        mIsGameOver(false),
        mOriginalActiveBlock(0),
        mQuality()
    {
    }


    int GameState::numLines() const
    {
        return mStats.mNumLines;
    }


    int GameState::numDoubles() const
    {
        return mStats.mNumDoubles;
    }


    int GameState::numTriples() const
    {        
        return mStats.mNumTriples;
    }


    int GameState::numTetrises() const
    {
        return mStats.mNumTetrises;
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
        if (mQuality.mDirty)
        {
            static const int cHolePenalty = 2;
            static const int cHeightPenalty = 1;
            int result = 0;
            mQuality.mNumHoles = 0;

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
                                mQuality.mNumHoles++;
                                result -= cHolePenalty;
                            }
                        }
                    }
                }
            }

            result -= static_cast<int>(mGrid.numRows() - top) * cHeightPenalty;
            float density = ((float)numOccupiedUnderTop) / (float)(((mGrid.numRows() - top) * mGrid.numColumns()));
            result += static_cast<int>((11.0*density) + 0.5);
            mQuality.mScore = result;
            mQuality.mDirty = false;
        }
        return mQuality.mScore;
    }


    int GameState::numHoles() const
    {
        return mQuality.mNumHoles;
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


    const ActiveBlock * GameState::originalActiveBlock() const
    {
        return mOriginalActiveBlock.get();
    }


    std::auto_ptr<GameState> GameState::clone() const
    {
        // We can't use the copy constructor because the std::auto_ptr member would invalidate itself on copy (!).
        std::auto_ptr<GameState> result(new GameState(mGrid.numRows(), mGrid.numColumns()));
        result->mGrid = mGrid;
        result->mIsGameOver = mIsGameOver;
        result->mStats = mStats;
        result->mQuality = mQuality;
        return result;
    }


    std::auto_ptr<GameState> GameState::commit(std::auto_ptr<ActiveBlock> inBlock, bool inGameOver) const
    {
        std::auto_ptr<GameState> result(clone());
        result->mIsGameOver = inGameOver;
        result->mQuality.mDirty = true;

        result->solidifyBlock(inBlock.get());

        // transfer ownership of the active block
        result->mOriginalActiveBlock = inBlock;

        // Clear lines and update score
        result->clearLines();

        return result;
    }


    void GameState::solidifyBlock(const ActiveBlock * inActiveBlock)
    {
        const Block & block = inActiveBlock->block();
        const Grid & grid = block.grid();
        for (size_t r = 0; r != grid.numRows(); ++r)
        {
            for (size_t c = 0; c != grid.numColumns(); ++c)
            {
                if (grid.get(r, c) != BlockType_Nil)
                {
                    mGrid.set(inActiveBlock->row() + r, inActiveBlock->column() + c, block.type());
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
            for (size_t rowIndex = mOriginalActiveBlock->row();
                 rowIndex < mOriginalActiveBlock->row() + mOriginalActiveBlock->block().grid().numRows();
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

        
        std::vector<BlockType> newData(mGrid.data().size(), BlockType_Nil);
        
        // Get newData
        {
            int newRowIdx = numLines;
            for (size_t r = 0; r != mGrid.numRows(); ++r)
            {                
                bool isLine = lines[r];
                for (size_t c = 0; c != mGrid.numColumns(); ++c)
                {
                    if (!isLine)
                    {
                        newData[newRowIdx * mGrid.numColumns() + c] = mGrid.get(r, c);
                    }
                }
                if (!isLine)
                {
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
                // Do nothing, mNumLines has already been incremented.
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

