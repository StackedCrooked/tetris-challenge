#include "TetrisComponent.h"
#include "TetrisPaintFunctions.h"
#include "Game.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/Gdiplus.h"
#include <boost/bind.hpp>
#include <set>


namespace Tetris
{

    typedef std::set<TetrisComponent*> TetrisComponentInstances;
    static TetrisComponentInstances sTetrisComponentInstances;
    static HHOOK sTetrisComponentInstances_KeyboardHook(0);


    XULWin::Component * CreateTetrisComponent(XULWin::Component * inParent, const XULWin::AttributesMapping & inAttr)
    {
        return new XULWin::Decorator(new TetrisComponent(inParent, inAttr));
    }


    TetrisComponent::TetrisComponent(XULWin::Component * inParent, const XULWin::AttributesMapping & inAttr) :
        XULWin::NativeControl(inParent, inAttr, TEXT("STATIC"), 0, 0),
        mGame(new Tetris::Game(20, 10, Tetris::BlockTypes()))
    {
        if (sTetrisComponentInstances.empty())
        {
            sTetrisComponentInstances_KeyboardHook = ::SetWindowsHookEx(WH_KEYBOARD,
                                                                        (HOOKPROC)TetrisComponent::KeyboardProc,
                                                                        ::GetModuleHandle(0),
                                                                        ::GetCurrentThreadId());
        }
        sTetrisComponentInstances.insert(this);
    }


    TetrisComponent::~TetrisComponent()
    {
        sTetrisComponentInstances.erase(this);
        if (sTetrisComponentInstances.empty())
        {
            ::UnhookWindowsHookEx(sTetrisComponentInstances_KeyboardHook);
        }
    }

    
    bool TetrisComponent::init()
    {
        return Super::init();
    }
        
        
    LRESULT TetrisComponent::onKeyDown(WPARAM wParam, LPARAM lParam)
    {        
        switch (wParam)
        {
            case VK_LEFT:
            {
                mGame->move(Direction_Left);
                break;
            }
            case VK_RIGHT:
            {
                mGame->move(Direction_Right);
                break;
            }
            case VK_UP:
            {
                mGame->rotate();
                break;
            }
            case VK_DOWN:
            {
                mGame->move(Direction_Down);
                break;
            }
            case VK_SPACE:
            {
                mGame->drop();
                break;
            }
            default:
            {
                return XULWin::cUnhandled;
            }
        }
        ::InvalidateRect(handle(), 0, FALSE);
        return XULWin::cHandled;
    }


    const Game & TetrisComponent::getGame() const
    {
        return *mGame;
    }


    Game & TetrisComponent::getGame()
    {
        return *mGame;
    }


    int TetrisComponent::calculateWidth(XULWin::SizeConstraint inSizeConstraint) const
    {
        if (!mGame)
        {
            return cUnitWidth * 10;
        }
        else
        {
            return cUnitWidth * mGame->currentNode()->state().grid().numColumns();
        }
    }


    int TetrisComponent::calculateHeight(XULWin::SizeConstraint inSizeConstraint) const
    {
        if (!mGame)
        {
            return cUnitWidth * 20;
        }
        else
        {
            return cUnitHeight * mGame->currentNode()->state().grid().numRows();
        }
    }


    void TetrisComponent::paint(HDC inHDC)
    {
        Gdiplus::Graphics g(inHDC);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
        g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
        const Grid & grid = mGame->currentNode()->state().grid();
        PaintGrid(g, grid, 0, 0);

        const Block & block = mGame->activeBlock();
        PaintGrid(g, block.grid(), block.column() * cUnitWidth, block.row() * cUnitHeight);
    }


    void TetrisComponent::bufferedPaint(HDC inHDC)
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


    LRESULT TetrisComponent::handleMessage(UINT inMessage, WPARAM wParam, LPARAM lParam)
    {
        if (inMessage == WM_PAINT)
        {
            HDC hDC = ::GetDC(handle());
            PAINTSTRUCT ps;
            ps.hdc = hDC;
            ::BeginPaint(handle(), &ps);
            bufferedPaint(hDC);
            ::EndPaint(handle(), &ps);
            ::ReleaseDC(handle(), hDC);
            return 0;
        }
        else if (inMessage == WM_KEYDOWN)
        {
            return onKeyDown(wParam, lParam);
        }
        return Super::handleMessage(inMessage, wParam, lParam);
    }


    LRESULT TetrisComponent::keyboardProc(int inCode, WPARAM wParam, LPARAM lParam)
    {
        if (inCode == HC_ACTION && !(HIWORD(lParam) & KF_UP))
        {
            onKeyDown(wParam, lParam);
        }
        return 0;
    }


    LRESULT CALLBACK TetrisComponent::KeyboardProc(int inCode, WPARAM wParam, LPARAM lParam)
    {
        TetrisComponentInstances::iterator it = sTetrisComponentInstances.begin(), end = sTetrisComponentInstances.end();
        for (; it != end; ++it)
        {
            TetrisComponent * pThis = *it;
            pThis->keyboardProc(inCode, wParam, lParam);
        }        
        return ::CallNextHookEx(sTetrisComponentInstances_KeyboardHook, inCode, wParam, lParam);
    }

    



} // namespace Tetris
