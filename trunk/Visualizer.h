#ifndef VISUALIZER_H
#define VISUALIZER_H


#include "PuzzleSolver.h"
#include <windows.h>
#include <gdiplus.h>


namespace Tetris
{

	class Visualizer
	{
	public:
		Visualizer(PuzzleSolver * inPuzzleSolver);

		~Visualizer();

		void show();

		void refresh();

	private:
		Visualizer(const Visualizer &);
		Visualizer & operator=(const Visualizer &);

		static void Initialize();
		static void Finalize();
		static LRESULT CALLBACK MessageHandler(HWND hWnd, UINT inMessage, WPARAM wParam, LPARAM lParam);

		static void CALLBACK TimerCallback(HWND hWnd, UINT inMessage, UINT_PTR inTimerID, DWORD inTime);

		void onPaint(HDC inHDC);

		void drawTree(Gdiplus::Graphics & inGraphics, const GameStateNode & inNode, const Gdiplus::RectF & inRect, int level);

		void drawText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Gdiplus::RectF & inRect, const Gdiplus::Brush & inBrush);
		
		void drawText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Gdiplus::RectF & inRect);

		void drawGrid(Gdiplus::Graphics & inGraphics, const Grid & inGrid, const Gdiplus::RectF & inRect);

		void drawRemainingBlocks(Gdiplus::Graphics & inGraphics, const Gdiplus::RectF & inRect);

		void drawState(Gdiplus::Graphics & inGraphics, const GameState & inState, const Gdiplus::RectF & inRect);

		static Gdiplus::Color GetColor(BlockType inType);

		HWND mHandle;
		HWND mUpButton;
		HWND mDownButton;
		typedef std::map<HWND, Visualizer*> Instances;

		PuzzleSolver * mPuzzleSolver;
		UINT_PTR mTimerID;
		int mDelay;

		static Instances sInstances;
		static ULONG_PTR sGdiPlusToken;
		static int sRefCount;
	};

} // namespace Tetris


#endif // VISUALIZER_H
