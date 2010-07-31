#include "Visualizer.h"
#include "GameState.h"
#include "Game.h"
#include <boost/bind.hpp>
#include <string>
#include <sstream>
#include <ctime>
#include <gdiplus.h>


namespace Tetris
{


    std::map<HWND, Visualizer *> Visualizer::sInstances;
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



    Visualizer::Visualizer(GameController * inGameController) :
        mGameController(inGameController),
        mDelay(100)
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

        mUpButton = CreateWindowEx
                    (
                        0,
                        L"BUTTON",
                        L"+",
                        BS_PUSHBUTTON | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
                        0,
                        10,
                        16,
                        16,
                        mHandle,
                        (HMENU)101,
                        ::GetModuleHandle(0),
                        NULL
                    );

        mDownButton = CreateWindowEx
                      (
                          0,
                          L"BUTTON",
                          L"-",
                          BS_PUSHBUTTON | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
                          0,
                          26,
                          16,
                          16,
                          mHandle,
                          (HMENU)102,
                          ::GetModuleHandle(0),
                          NULL
                      );

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


    static const int cBlockWidth = 10;
    static const int cBlockHeight = 10;


    void Visualizer::paintUnit(Gdiplus::Graphics & g, int x, int y, BlockType inBlockType)
    {
        Gdiplus::SolidBrush solidBrush(GetColor(inBlockType));
        g.FillRectangle(&solidBrush, Gdiplus::RectF(static_cast<float>(x), static_cast<float>(y), static_cast<float>(cBlockWidth), static_cast<float>(cBlockHeight)));
    }


    void Visualizer::clearScreen(Gdiplus::Graphics & g)
    {
        g.Clear(Gdiplus::Color::Black);
    }


    void Visualizer::paintGrid(Gdiplus::Graphics & g)
    {
        const Grid & grid = mGameController->game().currentNode().state().grid();
        int offsetX = 40;
        int offsetY = 40;
        for (size_t r = 0; r != grid.numRows(); ++r)
        {
            for (size_t c = 0; c != grid.numColumns(); ++c)
            {
                paintUnit(g, offsetX + c * cBlockHeight, offsetY + r * cBlockWidth, grid.get(r, c));
            }
        }

        const ActiveBlock & activeBlock = mGameController->game().activeBlock();
        const Grid & blockGrid = activeBlock.block().grid();
        for (size_t r = 0; r != blockGrid.numRows(); ++r)
        {
            for (size_t c = 0; c != blockGrid.numColumns(); ++c)
            {
                int x = offsetX + (c + activeBlock.column()) * cBlockWidth;
                int y = offsetY + (r + activeBlock.row()) * cBlockHeight;
                BlockType blockType = blockGrid.get(r, c);
                paintUnit(g, x, y, blockType);
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
        HBRUSH backgroundBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
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
        clearScreen(g);
        paintGrid(g);
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
        //mGameController->game().moveDown();
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
                    pThis->mDelay += 10;
                }
                else if (wParam == 102)
                {
                    if (pThis->mDelay > 10)
                    {
                        pThis->mDelay -= 10;
                    }
                }
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