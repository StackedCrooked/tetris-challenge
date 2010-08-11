#ifndef TETRISCOMPONENT_H_INCLUDED
#define TETRISCOMPONENT_H_INCLUDED


#include "XULWin/Component.h"
#include "XULWin/AttributeController.h"
#include "XULWin/Decorator.h"
#include "XULWin/EventListener.h"
#include "XULWin/GdiplusLoader.h"
#include "XULWin/NativeControl.h"
#include "XULWin/VirtualComponent.h"
#include <boost/scoped_ptr.hpp>


namespace Tetris
{
    class Game;


    /**
     * Attribute: numfutureblocks
     * Type: string (representing an integer)
     * Purpose: The number of future blocks to show in the Tetris component
     */
    class NumFutureBlocksController : public XULWin::AttributeController
    {
    public:
        static const char * AttributeName()
        {
            return "numfutureblocks";
        }

        virtual void get(std::string & outValue);

        virtual void set(const std::string & inValue);

        virtual int getNumFutureBlocks() const = 0;

        virtual void setNumFutureBlocks(int inNumFutureBlocks) = 0;
    };


    class TetrisComponent : public XULWin::NativeControl,
                            public NumFutureBlocksController,
                            public XULWin::GdiplusLoader
    {
    public:
        typedef XULWin::NativeControl Super;

        TetrisComponent(XULWin::Component * inParent, const XULWin::AttributesMapping & inAttr);

        virtual ~TetrisComponent();

        virtual bool init();

        const Game & getGame() const;

        Game & getGame();

        virtual bool initAttributeControllers();

        virtual int calculateWidth(XULWin::SizeConstraint inSizeConstraint) const;

        virtual int calculateHeight(XULWin::SizeConstraint inSizeConstraint) const;

        virtual LRESULT handleMessage(UINT inMessage, WPARAM wParam, LPARAM lParam);

        // NumFutureBlocksController
        virtual int getNumFutureBlocks() const;

        virtual void setNumFutureBlocks(int inNumFutureBlocks);

    private:
        LRESULT onKeyDown(WPARAM wParam, LPARAM lParam);

        static LRESULT CALLBACK KeyboardProc(int inCode, WPARAM wParam, LPARAM lParam);

        LRESULT keyboardProc(int inCode, WPARAM wParam, LPARAM lParam);

        void bufferedPaint(HDC inHDC);

        virtual void paint(HDC inHDC);

        boost::scoped_ptr<Game> mGame;
        int mNumFutureBlocks;
    };

} // namespace Tetris


#endif // TETRISCOMPONENT_H_INCLUDED
