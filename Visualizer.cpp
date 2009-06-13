#include "Visualizer.h"
#include <boost/bind.hpp>
#include <string>
#include <sstream>
#include <assert.h>


namespace Tetris
{


	std::map<HWND, Visualizer*> Visualizer::sInstances;
	ULONG_PTR Visualizer::sGdiPlusToken = 0;
	int Visualizer::sRefCount = 0;


	void Visualizer::Initialize()
	{
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup(&sGdiPlusToken, &gdiplusStartupInput, NULL);
	}


	void Visualizer::Finalize()
	{
		Gdiplus::GdiplusShutdown(sGdiPlusToken);
	}



	Visualizer::Visualizer(const GameState & inGameGrids) :
		mGameGrid(inGameGrids)
	{
		sRefCount++;
		if (sRefCount == 1)
		{
			Initialize();
		}
	}


	Visualizer::~Visualizer()
	{
		sRefCount--;
		if (sRefCount == 0)
		{
			Finalize();
		}
		::DestroyWindow(mHandle);
	}


	void Visualizer::show()
	{
		static TCHAR * fClassName = L"ShowBlocksWindow";
		static bool fClassRegistered = false;
		if (! fClassRegistered)
		{
			// initialize window		
			WNDCLASSEX wndClass;
			wndClass.cbSize = sizeof(WNDCLASSEX);
			wndClass.style = 0;
			wndClass.lpfnWndProc = &Visualizer::MessageHandler;
			wndClass.cbClsExtra	= 0;
			wndClass.cbWndExtra	= 0;
			wndClass.hInstance	= ::GetModuleHandle(0);
			wndClass.hIcon = 0;
			wndClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
			wndClass.hbrBackground = 0; // covered by content so no color needed (reduces flicker)
			wndClass.lpszMenuName = NULL;
			wndClass.lpszClassName = fClassName;
			wndClass.hIconSm = 0;
			RegisterClassEx(&wndClass);
			fClassRegistered = true;
		}
		
		mHandle = CreateWindowEx
		(
			0,
			fClassName,
			L"All blocks",
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
			(1280 - 640)/2,
			(1024 - 640)/2,
			640,
			640,
			NULL,
			NULL,
			::GetModuleHandle(0),
			NULL
		);
		if (!mHandle)
		{
			MessageBox(0, L"Failed to create window", L"Tetris Challenge", MB_OK);
			return;
		}

		sInstances.insert(std::make_pair(mHandle, this));

		::ShowWindow(mHandle, SW_SHOW);	
		MSG message;
		while (GetMessage(&message, NULL, 0, 0))
		{
			HWND hActive = GetActiveWindow();
			if (! IsDialogMessage(hActive, &message))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
	}

	void Visualizer::refresh()
	{
		::InvalidateRect(mHandle, 0, FALSE);
		::UpdateWindow(mHandle);
	}


	//void Visualizer::drawBlock(Gdiplus::Graphics & inGraphics, const Block & inBlock, const Gdiplus::RectF & inRect)
	//{
	//	Gdiplus::SolidBrush fgBrush(Gdiplus::Color::Blue);
	//	Gdiplus::SolidBrush bgBrush(Gdiplus::Color::Yellow);
	//	for (size_t i = 0; i != inBlock.grid().numRows(); ++i)
	//	{
	//		for (size_t j = 0; j != inBlock.grid().numColumns(); ++j)
	//		{
	//			Gdiplus::RectF rect(inRect.GetLeft() + j * cUnitWidth, inRect.GetTop() + i * cUnitHeight, cUnitWidth, cUnitHeight);
	//			inGraphics.FillRectangle((inBlock.grid().get(i, j) != 0) ? &fgBrush : &bgBrush, rect);
	//		}
	//	}
	//}


	void Visualizer::onPaint(HDC inHDC)
	{
		// Obtain the client rect of the window.
		RECT rc;
		::GetClientRect(mHandle, &rc);

		// Erase background
		Gdiplus::Graphics g(inHDC);
		Gdiplus::SolidBrush bgBrush(Gdiplus::Color::White);
		Gdiplus::RectF bgRect(0, 0, rc.right-rc.left, rc.bottom-rc.top);
		g.FillRectangle(&bgBrush, bgRect);

		// Paint contents
		const int cWidth = rc.right - rc.left;
		const int cHeight = rc.bottom - rc.top;
		const int cMarginLeft = static_cast<int>((0.1 * static_cast<double>(cWidth)) + 0.5);
		const int cMarginRight = static_cast<int>((0.1 * static_cast<double>(cWidth)) + 0.5);
		const int cMarginTop = static_cast<int>((0.1 * static_cast<double>(cHeight)) + 0.5);
		const int cMarginBottom = static_cast<int>((0.1 * static_cast<double>(cHeight)) + 0.5);
		const int cUnitWidth = (cWidth - cMarginLeft - cMarginRight)/mGameGrid.grid().numColumns();
		const int cUnitHeight = cUnitWidth;

		for (size_t rowIdx = 0; rowIdx != mGameGrid.grid().numRows(); ++rowIdx)
		{
			for (size_t colIdx = 0; colIdx != mGameGrid.grid().numColumns(); ++colIdx)
			{
				Gdiplus::SolidBrush fgBrush(GetColor(mGameGrid.grid().get(rowIdx, colIdx)));
				Gdiplus::RectF rect
				(
					cMarginLeft + colIdx * cUnitWidth,
					cMarginTop + rowIdx * cUnitHeight,
					cUnitWidth,
					cUnitHeight
				);
				g.FillRectangle(&fgBrush, rect);
			}
		}
		std::wstringstream ss;
		ss << L"Score: " << mGameGrid.calculateScore();
		std::wstring info(ss.str());

		Gdiplus::Font gdiFont(TEXT("Arial"), 9, Gdiplus::FontStyleRegular);			
		
		Gdiplus::RectF layoutRect
		(
			(float)cMarginLeft,
			(float)(cHeight - cMarginBottom/2),
			(float)(cWidth - cMarginLeft - cMarginRight),
			(float)cMarginBottom
		);
		Gdiplus::StringFormat stringFormat;
		Gdiplus::SolidBrush textBrush(Gdiplus::Color::Blue);
		g.DrawString(info.c_str(), (int)info.size(), &gdiFont, layoutRect, &stringFormat, &textBrush);

	}


	Gdiplus::Color Visualizer::GetColor(BlockType inType)
	{	
		switch (inType)
		{
			case I_BLOCK:
			{
				return Gdiplus::Color::Blue;
			}
			case J_BLOCK:
			{
				return Gdiplus::Color::Red;
			}
			case L_BLOCK:
			{
				return Gdiplus::Color::Yellow;
			}
			case O_BLOCK:
			{
				return Gdiplus::Color::Orange;
			}
			case S_BLOCK:
			{
				return Gdiplus::Color::Green;
			}
			case T_BLOCK:
			{
				return Gdiplus::Color::Pink;
			}
			case Z_BLOCK:
			{
				return Gdiplus::Color::Violet;
			}
			case NO_BLOCK:
			{
				return Gdiplus::Color::Gray;
			}
			default:
			{
				assert(!"No valid block type given.");
				return Gdiplus::Color::Black;
			}
		}
	}



	LRESULT CALLBACK Visualizer::MessageHandler(HWND hWnd, UINT inMessage, WPARAM wParam, LPARAM lParam)
	{	
		Instances::iterator it = sInstances.find(hWnd);
		if (it == sInstances.end())
		{
			return ::DefWindowProc(hWnd, inMessage, wParam, lParam);
		}

		Visualizer * pThis = it->second;

		switch (inMessage)
		{
			case WM_PAINT:
			{
				PAINTSTRUCT ps;
				::BeginPaint(hWnd, &ps);
				pThis->onPaint(ps.hdc);
				::EndPaint(hWnd, &ps);
				return TRUE;
			}
			case WM_ERASEBKGND:
			{
				return TRUE; // say we handled it
			}
			case WM_CLOSE:
			{
				::PostQuitMessage(0);
				return TRUE;
			}
		}
		return ::DefWindowProc(hWnd, inMessage, wParam, lParam);
	}


} // namespace Tetris