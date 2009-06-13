#include "GenericGrid.h"
#include "Parser.h"
#include "Visualizer.h"
#include "GameState.h"
#include <set>


namespace Tetris
{

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
				GameState gg;
				std::set<GameState> nextGrids;

				for (size_t rotIdx = 0; rotIdx != Block::NumRotations(static_cast<BlockType>(mBlocks[idx].type)); ++rotIdx)
				{
					gg.addBlock(Block::Get(static_cast<BlockType>(mBlocks[idx].type), rotIdx), nextGrids);
				}
				if (!nextGrids.empty())
				{
					mGameGrid = *nextGrids.begin();
				}
				mVisualizer.show();
			}	
		}
	private:
		GameState mGameGrid;
		Visualizer mVisualizer;
		std::vector<BlockIdentifier> mBlocks;
	};

} // namespace Tetris


INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Tetris::Game g;
	g.start();
	return 0;
}
