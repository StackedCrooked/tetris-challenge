#include "Visualizer.h"
#include <boost/bind.hpp>
#include <string>
#include <sstream>
#include <assert.h>
#include <ctime>


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



	Visualizer::Visualizer(PuzzleSolver * inPuzzleSolver) :
		mPuzzleSolver(inPuzzleSolver)
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

		mNextButton = CreateWindowEx
		(
			0,
			L"BUTTON",
			L"Next",
			BS_PUSHBUTTON | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
			10,
			10,
			80,
			22,
			mHandle,
			(HMENU)101,
			::GetModuleHandle(0),
			NULL
		);

		sInstances.insert(std::make_pair(mHandle, this));
		mTimerID = SetTimer(NULL, NULL, 500, &Visualizer::TimerCallback);
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


	void Visualizer::drawTree(Gdiplus::Graphics & inGraphics, const GameStateNode & inNode, const Gdiplus::RectF & inRect, int level)
	{
		int xOffset = inRect.X;
		int yOffset = inRect.Y;

		int score = inNode.state().calculateScore();
		std::wstringstream ss;
		ss << "Level: " << level << ". Score: " << score;
		Gdiplus::SolidBrush textBrush(Gdiplus::Color::Green);
		drawText(inGraphics, ss.str(), Gdiplus::RectF(xOffset, yOffset, 80, 30), textBrush);

		xOffset += 80;
		GameStateNode::Children::const_iterator it = inNode.children().begin(), end = inNode.children().end();
		for (; it != end; ++it)
		{
			if (*it)
			{
				GameStateNode & node = *it->get();
				std::wstringstream ss;
				ss << node.state().calculateScore();
				Gdiplus::SolidBrush textBrush(node.state().isDeadEnd() ? Gdiplus::Color::Red : node.children().empty() ? Gdiplus::Color::Gray : Gdiplus::Color::Green);
				drawText(inGraphics, ss.str(), Gdiplus::RectF(xOffset, yOffset, 30, 30), textBrush);
				yOffset += 30;

				Gdiplus::RectF rect(inRect.X + xOffset + 30, inRect.Y, inRect.Width - xOffset - 30, inRect.Height);
				drawTree(inGraphics, node, rect, level + 1);
			}
		}
	}


	void Visualizer::drawText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Gdiplus::RectF & inRect)
	{
		Gdiplus::SolidBrush textBrush(Gdiplus::Color::White);
		drawText(inGraphics, inText, inRect, textBrush);
	}

	void Visualizer::drawText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Gdiplus::RectF & inRect, const Gdiplus::Brush & inBrush)
	{
		Gdiplus::Font gdiFont(TEXT("Arial"), 9, Gdiplus::FontStyleRegular);			
		
		Gdiplus::RectF layoutRect
		(
			(float)inRect.X,
			(float)inRect.Y,
			(float)inRect.Width,
			(float)inRect.Height
		);
		Gdiplus::StringFormat stringFormat;
		inGraphics.DrawString(inText.c_str(), (int)inText.size(), &gdiFont, layoutRect, &stringFormat, &inBrush);
	}


	void Visualizer::drawState(Gdiplus::Graphics & inGraphics, const GameState & inState, const Gdiplus::RectF & inRect)
	{
		const Grid & grid = inState.grid();
		const int cUnitWidth = inRect.Width / 15;
		const int cUnitHeight = cUnitWidth;
		for (size_t rowIdx = 0; rowIdx != grid.numRows(); ++rowIdx)
		{
			for (size_t colIdx = 0; colIdx != grid.numColumns(); ++colIdx)
			{
				Gdiplus::SolidBrush fgBrush(GetColor(grid.get(rowIdx, colIdx)));
				Gdiplus::RectF rect
				(
					inRect.X + colIdx * cUnitWidth,
					inRect.Y + rowIdx * cUnitHeight,
					cUnitWidth,
					cUnitHeight
				);
				inGraphics.FillRectangle(&fgBrush, rect);
			}
		}
		std::wstringstream ss;
		ss << L"Score: " << inState.calculateScore();
		std::wstring info(ss.str());
		drawText(inGraphics, info, Gdiplus::RectF(inRect.X, inRect.Y + inRect.Height - 20, inRect.Width, inRect.Height));
	}


	void Visualizer::onPaint(HDC inHDC)
	{
		// Obtain the client rect of the window.
		RECT rc;
		::GetClientRect(mHandle, &rc);
		const int cWidth = rc.right - rc.left;
		const int cHeight = rc.bottom - rc.top;
		const int cMarginLeft = 20;
		const int cMarginRight = 20;
		const int cMarginTop = 40;
		const int cMarginBottom = 20;
		const int cRight = cWidth - cMarginRight;
		const int cUnitWidth = 8;
		const int cUnitHeight = cUnitWidth;
		const int cSpacing = 10;
		const int cTreeHeight = 0*400;

		// Erase background
		Gdiplus::Graphics g(inHDC);
		Gdiplus::SolidBrush bgBrush(Gdiplus::Color::Black);
		Gdiplus::RectF bgRect(0, 0, rc.right-rc.left, rc.bottom-rc.top);
		g.FillRectangle(&bgBrush, bgRect);

		// Paint contents
		const GameStateNode * node = mPuzzleSolver->currentNode()->parent();

		if (node)
		{
			int offsetX = cMarginLeft;
			int offsetY = cMarginTop;

			//drawTree(g, *node, Gdiplus::RectF(offsetX, offsetY, cWidth - cMarginLeft - cMarginRight, cTreeHeight), 0);
			offsetY += cTreeHeight;
			
			const GameStateNode::Children & children = node->children();
			GameStateNode::Children::const_iterator it = children.begin(), end = children.end();
			int count = 0;
			size_t numChildren = 1;//children.size();
			for (; it != end && count < numChildren; ++it)
			{
				int rectWidth = it->get()->state().grid().numColumns()*cUnitWidth;
				int rectHeight = rectWidth + 20;
				Gdiplus::RectF rect(offsetX, offsetY, rectWidth, rectHeight);
				drawState(g, it->get()->state(), rect);
				count++;
				offsetX += rectWidth + cSpacing;
				if (offsetX + rectWidth + cMarginRight > cWidth)
				{
					offsetX = cMarginLeft;
					offsetY += rectHeight + cSpacing;
				}
			}
		}
		std::wstringstream ss;
		ss << "Depth: " << mPuzzleSolver->depth() << "/" << mPuzzleSolver->blocks().size();
		drawText(g, ss.str(), Gdiplus::RectF(100, 10, cWidth - 100, 20));

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
				return Gdiplus::Color::Black;
			}
			default:
			{
				assert(!"No valid block type given.");
				return Gdiplus::Color::Black;
			}
		}
	}


	void CALLBACK Visualizer::TimerCallback(HWND hWnd, UINT inMessage, UINT_PTR inTimerID, DWORD inTime)
	{
		Instances::iterator it = sInstances.begin();
		if (it == sInstances.end())
		{
			return;
		}

		Visualizer * pThis = it->second;

		if (inTimerID == pThis->mTimerID)
		{
			if (pThis->mPuzzleSolver->depth() < 56)
			{
				pThis->mPuzzleSolver->next();
				::InvalidateRect(pThis->mHandle, 0, FALSE);
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
			case WM_SIZE:
			{
				::InvalidateRect(hWnd, 0, FALSE);
				break;
			}
			case WM_COMMAND:
			{
				if (wParam == 101)
				{
					pThis->mPuzzleSolver->next();
					::InvalidateRect(pThis->mHandle, 0, FALSE);
					::UpdateWindow(pThis->mHandle);
				}
				break;
			}
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