#ifndef TETRIS_GAMESTATE_H_INCLUDED
#define TETRIS_GAMESTATE_H_INCLUDED


#include "Tetris/Block.h"
#include "Tetris/GameOver.h"
#include "Tetris/Grid.h"
#include <memory>
#include <stdexcept>


namespace Tetris
{

    // Forward declarations    
    class Evaluator;

	
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


    class GameState
    {
    public:
        // Creates a new GameState
        GameState(size_t inNumRows, size_t inNumColumns);

        const Grid & grid() const;

        Grid & grid();

        // The Block that was used in the commit(...) call.
        const Block & originalBlock() const;

        bool isGameOver() const;

        // Calculates the quality of the playing field.
        // Caches the value.
        int quality(const Evaluator & inEvaluator) const;

        // Checks if a activeBlock can be placed at a given location without
        // overlapping with previously placed blocks.
        bool checkPositionValid(const Block & inBlock, size_t inRowIdx, size_t inColIdx) const;

        // Creates a copy of the current gamestate with the given active block committed.
        // Use inGameOver = true to mark the new gamestate as "game over".
        std::auto_ptr<GameState> commit(const Block & inBlock, GameOver inGameOver) const;
		
		// Statistics
		int numLines() const { return mNumLines; }
		int numSingles() const { return mNumSingles; }
		int numDoubles() const { return mNumDoubles; }
		int numTriples() const { return mNumTriples; }
		int numTetrises() const { return mNumTetrises; }
		int score() const;
		int firstOccupiedRow() const { return mFirstOccupiedRow; }


    private:
        void solidifyBlock(const Block & inBlock);        
        void clearLines();

        Grid mGrid;
        Block mOriginalBlock;
        bool mIsGameOver;
		int mFirstOccupiedRow;
		int mNumLines;
        int mNumSingles;
        int mNumDoubles;
        int mNumTriples;
        int mNumTetrises;
        mutable Quality mQuality;
    };


} // namespace Tetris


#endif // GAMESTATE_H_INCLUDED
