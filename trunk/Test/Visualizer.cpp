#include "Visualizer.h"
#include "GameState.h"
#include "Game.h"
#include "Unicode.h"
#include <boost/bind.hpp>
#include <string>
#include <sstream>
#include <ctime>
#include <gdiplus.h>


namespace Tetris
{


    Rect::Rect(int x, int y, int w, int h) :
        mX(x), mY(y), mWidth(w), mHeight(h)
    {
    }


    int Rect::x() const
    {
        return mX;
    }


    int Rect::y() const
    {
        return mY;
    }


    int Rect::width() const
    {
        return mWidth;
    }


    int Rect::height() const
    {
        return mHeight;
    }


    std::map<HWND, Visualizer *> Visualizer::sInstances;
    ULONG_PTR Visualizer::sGdiPlusToken = 0;
    int Visualizer::sRefCount = 0;


    clock_t GetClockMs()
    {
        return std::clock() * CLOCKS_PER_SEC / 1000;
    }


    void Visualizer::Initialize()
    {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&sGdiPlusToken, &gdiplusStartupInput, NULL);
    }


    void Visualizer::Finalize()
    {
        Gdiplus::GdiplusShutdown(sGdiPlusToken);
    }



    Visualizer::Visualizer(GameController * inGameController) :
        mGameController(inGameController),
        mHandle(0),
        mKeyboardHook(0),
        mDelay(10),
        mElapsed(GetClockMs())
    {
        sRefCount++;
        if (sRefCount == 1)
        {
            Initialize();
        }
    }


    Visualizer::~Visualizer()
    {
        if (mKeyboardHook)
        {
            ::UnhookWindowsHookEx(mKeyboardHook);
        }


        sRefCount--;
        if (sRefCount == 0)
        {
            Finalize();
        }
        ::DestroyWindow(mHandle);
    }


    void Visualizer::show()
    {
        static TCHAR * fClassName = L"VisualizerWindow";
        static bool fClassRegistered = false;
        if (! fClassRegistered)
        {
            // initialize window
            WNDCLASSEX wndClass;
            wndClass.cbSize = sizeof(WNDCLASSEX);
            wndClass.style = 0;
            wndClass.lpfnWndProc = &Visualizer::MessageHandler;
            wndClass.cbClsExtra = 0;
            wndClass.cbWndExtra = 0;
            wndClass.hInstance  = ::GetModuleHandle(0);
            wndClass.hIcon = 0;
            wndClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
            wndClass.hbrBackground = 0; // covered by content so no color needed (reduces flicker)
            wndClass.lpszMenuName = NULL;
            wndClass.lpszClassName = fClassName;
            wndClass.hIconSm = 0;
            if (RegisterClassEx(&wndClass) == 0)
            {
                throw std::runtime_error("RegisterClassEx failed.");
            }
            fClassRegistered = true;
        }

        mHandle = CreateWindowEx(0,
                                 fClassName,
                                 L"Tetris",
                                 WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW,
                                 (1280 - 640)/2,
                                 (1024 - 640)/2,
                                 640,
                                 640,
                                 NULL,
                                 NULL,
                                 ::GetModuleHandle(0),
                                 NULL);
        
        if (!mHandle)
        {
            throw std::runtime_error("Failed to create the main window");
        }

        // Hook keyboard
        mKeyboardHook = ::SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)Visualizer::KeyboardProc, ::GetModuleHandle(0), 0);

        sInstances.insert(std::make_pair(mHandle, this));
        mTimerID = SetTimer(NULL, NULL, mDelay, &Visualizer::TimerCallback);
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


    HWND Visualizer::handle()
    {
        return mHandle;
    }


    void Visualizer::refresh()
    {
        ::InvalidateRect(mHandle, 0, FALSE);
        ::UpdateWindow(mHandle);
    }


    Gdiplus::RectF ToRectF(const Rect & inRect)
    {
        return Gdiplus::RectF(static_cast<float>(inRect.x()),
                              static_cast<float>(inRect.y()),
                              static_cast<float>(inRect.width()),
                              static_cast<float>(inRect.height()));
    }


    void Visualizer::drawText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Rect & inRect)
    {
        Gdiplus::SolidBrush textBrush(Gdiplus::Color::Green);
        drawText(inGraphics, inText, inRect, textBrush);
    }

    void Visualizer::drawText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Rect & inRect, const Gdiplus::Brush & inBrush)
    {
        Gdiplus::Font gdiFont(TEXT("Arial"), 9, Gdiplus::FontStyleRegular);

        Gdiplus::StringFormat stringFormat;
        inGraphics.DrawString(inText.c_str(), (int)inText.size(), &gdiFont, ToRectF(inRect), &stringFormat, &inBrush);
    }


    static const int cBlockWidth = 20;
    static const int cBlockHeight = 20;
    static const int cGridOffsetX = 40;
    static const int cGridOffsetY = 40;
    static const int cSpacing = 20;
    static const int cScoresOffsetX = cGridOffsetX + 10 * cBlockWidth + cSpacing;
    static const int cScoresOffsetY = cGridOffsetY;


    void Visualizer::paintUnit(Gdiplus::Graphics & g, int x, int y, BlockType inBlockType)
    {
        Gdiplus::SolidBrush solidBrush(GetColor(inBlockType));
        g.FillRectangle(&solidBrush, Gdiplus::RectF(static_cast<float>(x), static_cast<float>(y), static_cast<float>(cBlockWidth), static_cast<float>(cBlockHeight)));
    }


    static std::wstring GetScoreText(const std::string & inTitle, int inScore)
    {
        std::stringstream ss;
        ss << inTitle << ": " << inScore;
        return ToUTF16(ss.str());
    }


    void Visualizer::paintScores(Gdiplus::Graphics & g)
    {
        const GameState::Stats & stats = mGameController->game().currentNode().state().stats();
        drawText(g, GetScoreText("Lines", stats.mNumLines), Rect(cScoresOffsetX, cScoresOffsetY, 200, 40));
        drawText(g, GetScoreText("x1", stats.mNumSingles), Rect(cScoresOffsetX, cScoresOffsetY + 40, 200, 40));
        drawText(g, GetScoreText("x2", stats.mNumDoubles), Rect(cScoresOffsetX, cScoresOffsetY + 80, 200, 40));
        drawText(g, GetScoreText("x3", stats.mNumTriples), Rect(cScoresOffsetX, cScoresOffsetY + 120, 200, 40));
        drawText(g, GetScoreText("x4", stats.mNumTetrises), Rect(cScoresOffsetX, cScoresOffsetY + 160, 200, 40));
    }


    void Visualizer::paintGrid(Gdiplus::Graphics & g)
    {
        const Grid & grid = mGameController->game().currentNode().state().grid();
        int cGridOffsetX = 40;
        int cGridOffsetY = 40;
        for (size_t r = 0; r != grid.numRows(); ++r)
        {
            for (size_t c = 0; c != grid.numColumns(); ++c)
            {
                paintUnit(g, cGridOffsetX + c * cBlockHeight, cGridOffsetY + r * cBlockWidth, grid.get(r, c));
            }
        }

        const Block & block = mGameController->game().activeBlock();
        const Grid & blockGrid = block.grid();
        for (size_t r = 0; r != blockGrid.numRows(); ++r)
        {
            for (size_t c = 0; c != blockGrid.numColumns(); ++c)
            {
                int x = cGridOffsetX + (c + block.column()) * cBlockWidth;
                int y = cGridOffsetY + (r + block.row()) * cBlockHeight;
                if (BlockType blockType = blockGrid.get(r, c))
                {
                    paintUnit(g, x, y, blockType);
                }
            }
        }
    }


    void Visualizer::bufferedPaint(HDC inHDC)
    {

        //
        // Get the size of the client rectangle.
        //
        RECT rc;
        GetClientRect(handle(), &rc);

        HDC compatibleDC = CreateCompatibleDC(inHDC);


        //
        // Create a bitmap big enough for our client rectangle.
        //
        HBITMAP backgroundBuffer = CreateCompatibleBitmap(inHDC, rc.right - rc.left, rc.bottom - rc.top);


        //
        // Select the bitmap into the off-screen DC.
        //
        HBITMAP backgroundBitmap = (HBITMAP)SelectObject(compatibleDC, backgroundBuffer);


        //
        // Erase the background.
        //
        HBRUSH backgroundBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(compatibleDC, &rc, backgroundBrush);
        DeleteObject(backgroundBrush);

        //
        // Render the image into the offscreen DC.
        //
        SetBkMode(compatibleDC, TRANSPARENT);


        paint(compatibleDC);


        //
        // Blt the changes to the screen DC.
        //
        BitBlt
        (
            inHDC,
            rc.left,
            rc.top,
            rc.right - rc.left,
            rc.bottom - rc.top,
            compatibleDC, 0, 0, SRCCOPY
        );

        //
        // Done with off-screen bitmap and DC.
        //
        SelectObject(compatibleDC, backgroundBitmap);
        DeleteObject(backgroundBuffer);
        DeleteDC(compatibleDC);
    }


    void Visualizer::paint(HDC inHDC)
    {
        Gdiplus::Graphics g(inHDC);
        paintGrid(g);
        paintScores(g);
    }


    const Gdiplus::Color & Visualizer::GetColor(BlockType inBlockType)
    {
        if (inBlockType < BlockType_Nil || inBlockType >= BlockType_End)
        {
            throw std::logic_error("Invalid BlockType enum value.");
        }

        static const Gdiplus::Color fColors[] =
        {
            Gdiplus::Color(Gdiplus::Color::DarkGray),
            Gdiplus::Color(Gdiplus::Color::Violet),
            Gdiplus::Color(Gdiplus::Color::Blue),
            Gdiplus::Color(Gdiplus::Color::Orange),
            Gdiplus::Color(Gdiplus::Color::Yellow),
            Gdiplus::Color(Gdiplus::Color::LightGreen),
            Gdiplus::Color(Gdiplus::Color::Blue),
            Gdiplus::Color(Gdiplus::Color::Red)
        };

        return fColors[static_cast<int>(inBlockType)];
    }


    void Visualizer::timerCallback()
    {
        clock_t elapsed = GetClockMs();
        if (elapsed - mElapsed > 500)
        {
            mGameController->game().move(Direction_Down);
            mElapsed = elapsed;
        }
        ::InvalidateRect(mHandle, 0, FALSE);
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
            pThis->timerCallback();

            // Reset timer because mDelay might have been changed.
            ::KillTimer(0, pThis->mTimerID);
            pThis->mTimerID = ::SetTimer(0, 0, pThis->mDelay, &Visualizer::TimerCallback);
        }
    }


    LRESULT CALLBACK Visualizer::KeyboardProc(int inCode, WPARAM wParam, LPARAM lParam)
    {
        Visualizer * pThis = sInstances.begin()->second; // i suck


        if (inCode == HC_ACTION && !(HIWORD(lParam) & KF_UP))
        {
            switch (wParam)
            {
                case VK_LEFT:
                {
                    pThis->mGameController->move(Direction_Left);
                    break;
                }
                case VK_RIGHT:
                {
                    pThis->mGameController->move(Direction_Right);
                    break;
                }
                case VK_UP:
                {
                    pThis->mGameController->rotate();
                    break;
                }
                case VK_DOWN:
                {
                    pThis->mGameController->move(Direction_Down);
                    break;
                }
                case VK_SPACE:
                {
                    pThis->mGameController->drop();
                    break;
                }
            }
        }
        
        return ::CallNextHookEx(pThis->mKeyboardHook, inCode, wParam, lParam);
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
            case WM_PAINT:
            {                
                HDC hDC = ::GetDC(hWnd);
                PAINTSTRUCT ps;
                ps.hdc = hDC;
                ::BeginPaint(hWnd, &ps);
                pThis->bufferedPaint(hDC);
                ::EndPaint(hWnd, &ps);
                ::ReleaseDC(hWnd, hDC);                
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
