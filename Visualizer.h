#ifndef VISUALIZER_H
#define VISUALIZER_H


#include "Block.h"
#include "GameState.h"
#include <map>
#include <set>
#include <windows.h>
#include <gdiplus.h>


namespace Tetris
{

	class Visualizer
	{
	public:
		Visualizer(const GameState & inGameGrids);

		~Visualizer();

		void show();

		void refresh();

		//void drawBlock(Gdiplus::Graphics & inGraphics, const Block & inBlock, const Gdiplus::RectF & inRect);

	private:
		Visualizer(const Visualizer &);
		Visualizer & operator=(const Visualizer &);

		static void Initialize();
		static void Finalize();
		static LRESULT CALLBACK MessageHandler(HWND hWnd, UINT inMessage, WPARAM wParam, LPARAM lParam);

		void onPaint(HDC inHDC);

		static Gdiplus::Color GetColor(BlockType inType);

		HWND mHandle;
		typedef std::map<HWND, Visualizer*> Instances;

		const GameState & mGameGrid;

		static Instances sInstances;
		static ULONG_PTR sGdiPlusToken;
		static int sRefCount;
	};

} // namespace Tetris


#endif // VISUALIZER_H
