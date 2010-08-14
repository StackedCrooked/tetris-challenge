#include "TetrisComponent.h"
#include "Player.h"
#include "TetrisPaintFunctions.h"
#include "XULWin/Conversions.h"
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

    
    void NumFutureBlocksController::get(std::string & outValue)
    {
        outValue = XULWin::Int2String(getNumFutureBlocks());
    }

    
    void NumFutureBlocksController::set(const std::string & inValue)
    {
        setNumFutureBlocks(XULWin::String2Int(inValue, 1));
    }

    
    void KeyboardEnabledController::get(std::string & outValue)
    {
        outValue = XULWin::Bool2String(getKeyboardEnabled());
    }

    
    void KeyboardEnabledController::set(const std::string & inValue)
    {
        setKeyboardEnabled(XULWin::String2Bool(inValue, true));
    }


    TetrisComponent::TetrisComponent(XULWin::Component * inParent, const XULWin::AttributesMapping & inAttr) :
        XULWin::NativeControl(inParent, inAttr, TEXT("STATIC"), 0, 0),
        mThreadSafeGame(new ThreadSafeGame(std::auto_ptr<Game>(new Tetris::Game(20, 10, Tetris::BlockTypes())))),
        mNumFutureBlocks(1),
        mKeyboardEnabled(true)
    {
        if (sTetrisComponentInstances.empty())
        {
            sTetrisComponentInstances_KeyboardHook = ::SetWindowsHookEx(WH_KEYBOARD,
                                                                        (HOOKPROC)TetrisComponent::KeyboardProc,
                                                                        ::GetModuleHandle(0),
                                                                        ::GetCurrentThreadId());
        }
        sTetrisComponentInstances.insert(this);

        mWinAPITimer.start(boost::bind(&TetrisComponent::onTimerEvent, this), 10);
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


    void TetrisComponent::onTimerEvent()
    {
        ::InvalidateRect(handle(), 0, FALSE);
    }

    
    int TetrisComponent::getNumFutureBlocks() const
    {
        return mNumFutureBlocks;
    }


    void TetrisComponent::setNumFutureBlocks(int inNumFutureBlocks)
    {
        mNumFutureBlocks = inNumFutureBlocks;
    }


    bool TetrisComponent::getKeyboardEnabled() const
    {
        return mKeyboardEnabled;
    }

    void TetrisComponent::setKeyboardEnabled(bool inKeyboardEnabled)
    {
        mKeyboardEnabled = inKeyboardEnabled;
    }
        
        
    LRESULT TetrisComponent::onKeyDown(WPARAM wParam, LPARAM lParam)
    {       
        // Scoped lock
        {
            switch (wParam)
            {
                case VK_LEFT:
                {   
                    WritableGame game(*mThreadSafeGame);
                    game->move(Direction_Left);
                    break;
                }
                case VK_RIGHT:
                {
                    WritableGame game(*mThreadSafeGame);
                    game->move(Direction_Right);
                    break;
                }
                case VK_UP:
                {
                    WritableGame game(*mThreadSafeGame);
                    game->rotate();
                    break;
                }
                case VK_DOWN:
                {
                    WritableGame game(*mThreadSafeGame);
                    game->move(Direction_Down);
                    break;
                }
                case VK_SPACE:
                {
                    WritableGame game(*mThreadSafeGame);
                    game->drop();
                    break;
                }
                case VK_DELETE:
                {
                    Tetris::Player player(getThreadSafeGame());
                    Tetris::Widths widths;
                    widths.push_back(4);
                    widths.push_back(4);
                    widths.push_back(4);
                    player.move(widths);
                    break;
                }
                default:
                {
                    return XULWin::cUnhandled;
                }
            }
        }
        ::InvalidateRect(handle(), 0, FALSE);
        return XULWin::cHandled;
    }


    const ThreadSafeGame & TetrisComponent::getThreadSafeGame() const
    {
        return *mThreadSafeGame;
    }


    ThreadSafeGame & TetrisComponent::getThreadSafeGame()
    {
        return *mThreadSafeGame;
    }


    bool TetrisComponent::initAttributeControllers()
    {
        setAttributeController<NumFutureBlocksController>(this);
        setAttributeController<KeyboardEnabledController>(this);
        return Super::initAttributeControllers();
    }


    int TetrisComponent::calculateWidth(XULWin::SizeConstraint inSizeConstraint) const
    {
        int result = 0;
        if (!mThreadSafeGame)
        {
            result += cUnitWidth * 10;
        }
        else
        {
            ReadOnlyGame game(*mThreadSafeGame);
            result += cUnitWidth * game->numColumns();
        }

        if (getNumFutureBlocks() > 0)
        {
            result += 5 * cUnitWidth;
        }

        return result;
    }


    int TetrisComponent::calculateHeight(XULWin::SizeConstraint inSizeConstraint) const
    {
        int result = 0;
        if (!mThreadSafeGame)
        {
            result += cUnitWidth * 20;
        }
        else
        {
            ReadOnlyGame game(*mThreadSafeGame);
            result += cUnitHeight * game->numRows();
        }

        int requiredHeightForFutureBlocks = cUnitHeight + (5 * getNumFutureBlocks() * cUnitHeight);
        if (requiredHeightForFutureBlocks > result)
        {
            result = requiredHeightForFutureBlocks;
        }
        return result;
    }


    void TetrisComponent::paint(HDC inHDC)
    {
        Gdiplus::Graphics g(inHDC);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQuality);
        g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

        // Scoped lock
        {
            ReadOnlyGame game(*mThreadSafeGame);

            // Paint the current game grid
            const Grid & grid = game->currentNode()->state().grid();
            PaintGrid(g, grid, 0, 0, true);

            // Paint the currently active block
            const Block & block = game->activeBlock();
            PaintGrid(g, block.grid(), block.column() * cUnitWidth, block.row() * cUnitHeight, false);

            // Paint future blocks
            BlockTypes futureBlockTypes;
            game->getFutureBlocks(mNumFutureBlocks + 1, futureBlockTypes);
            for (size_t idx = 1; idx < futureBlockTypes.size(); ++idx)
            {
                int x = cUnitWidth * (game->numColumns() + 1);
                int y = cUnitHeight + (idx - 1) * (5 * cUnitHeight);
                PaintGrid(g, GetGrid(GetBlockIdentifier(futureBlockTypes[idx], 0)), x, y, false);
            }
        }
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
        if (mKeyboardEnabled && inCode == HC_ACTION && !(HIWORD(lParam) & KF_UP))
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
