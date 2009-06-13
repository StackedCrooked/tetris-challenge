#include "GenericGrid.h"
#include "Parser.h"
#include "Visualizer.h"
#include "GameGrid.h"
#include <set>

typedef GenericGrid<char> Grid;


class Game
{
public:
	Game() :
	  mVisualizer(mGameGrid)
	{
		Parser p;
		p.parse("inputs.txt", mBlocks);
	}

	void start()
	{
		for (size_t idx = 0; idx != mBlocks.size(); ++idx)
		{
			GameGrid gg;
			std::set<GameGrid> nextGrids;

			for (size_t rotIdx = 0; rotIdx != Block::NumRotations(static_cast<Block::Type>(mBlocks[idx].type)); ++rotIdx)
			{
				gg.addBlock(Block::Get(static_cast<Block::Type>(mBlocks[idx].type), rotIdx), nextGrids);
			}
			if (!nextGrids.empty())
			{
				mGameGrid = *nextGrids.begin();
			}
			mVisualizer.show();
		}	
	}
private:
	GameGrid mGameGrid;
	Visualizer mVisualizer;
	std::vector<BlockIdentifier> mBlocks;
};


INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Game g;
	g.start();
	return 0;
}
