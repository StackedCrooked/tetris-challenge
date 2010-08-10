#ifndef TETRISCOMPONENT_H_INCLUDED
#define TETRISCOMPONENT_H_INCLUDED


#include "XULWin/Component.h"
#include "XULWin/Decorator.h"
#include "XULWin/EventListener.h"
#include "XULWin/GdiplusLoader.h"
#include "XULWin/NativeControl.h"
#include "XULWin/VirtualComponent.h"
#include <boost/scoped_ptr.hpp>


namespace Tetris
{
    class Game;

    class TetrisComponent : public XULWin::NativeControl,
                            public XULWin::GdiplusLoader
    {
    public:
        typedef XULWin::NativeControl Super;

        TetrisComponent(XULWin::Component * inParent, const XULWin::AttributesMapping & inAttr);

        virtual ~TetrisComponent();

        virtual bool init();

        const Game & getGame() const;

        Game & getGame();

        virtual int calculateWidth(XULWin::SizeConstraint inSizeConstraint) const;

        virtual int calculateHeight(XULWin::SizeConstraint inSizeConstraint) const;

        virtual LRESULT handleMessage(UINT inMessage, WPARAM wParam, LPARAM lParam);

    private:
        LRESULT onKeyDown(WPARAM wParam, LPARAM lParam);

        static LRESULT CALLBACK KeyboardProc(int inCode, WPARAM wParam, LPARAM lParam);

        LRESULT keyboardProc(int inCode, WPARAM wParam, LPARAM lParam);

        void bufferedPaint(HDC inHDC);

        virtual void paint(HDC inHDC);

        boost::scoped_ptr<Game> mGame;
    };

    
} // namespace Tetris


#endif // TETRISCOMPONENT_H_INCLUDED
