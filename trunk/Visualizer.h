#ifndef VISUALIZER_H
#define VISUALIZER_H


#include "Block.h"
#include "GameGrid.h"
#include <map>
#include <set>
#include <windows.h>
#include <gdiplus.h>


class Visualizer
{
public:
	Visualizer(const GameGrid & inGameGrids);

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

	static Gdiplus::Color GetColor(Block::Type inType);

	HWND mHandle;
	typedef std::map<HWND, Visualizer*> Instances;

	const GameGrid & mGameGrid;

	static Instances sInstances;
	static ULONG_PTR sGdiPlusToken;
	static int sRefCount;
};


#endif // VISUALIZER_H
