#ifndef TETRISCOMPONENT_H_INCLUDED
#define TETRISCOMPONENT_H_INCLUDED


#include "Game.h"
#include "ThreadSafeGame.h"
#include "XULWin/Component.h"
#include "XULWin/AttributeController.h"
#include "XULWin/Decorator.h"
#include "XULWin/EventListener.h"
#include "XULWin/GdiplusLoader.h"
#include "XULWin/NativeControl.h"
#include "XULWin/VirtualComponent.h"
#include "XULWin/WinUtils.h"
#include "Poco/Stopwatch.h"
#include <boost/scoped_ptr.hpp>
#include <boost/signals.hpp>


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


    /**
     * Attribute: keyboardenabled
     * Type: string (representing an integer)
     * Purpose: The number of future blocks to show in the Tetris component
     */
    class KeyboardEnabledController : public XULWin::AttributeController
    {
    public:
        static const char * AttributeName()
        {
            return "keyboardenabled";
        }

        virtual void get(std::string & outValue);

        virtual void set(const std::string & inValue);

        virtual bool getKeyboardEnabled() const = 0;

        virtual void setKeyboardEnabled(bool inKeyboardEnabled) = 0;
    };


    class TetrisComponent : public XULWin::NativeControl,
                            public NumFutureBlocksController,
                            public KeyboardEnabledController,
                            public XULWin::GdiplusLoader
    {
    public:
        typedef XULWin::NativeControl Super;

        TetrisComponent(XULWin::Component * inParent, const XULWin::AttributesMapping & inAttr);

        virtual ~TetrisComponent();

        boost::signal<void(int)> OnKeyboardPressed;

        virtual bool init();

        inline int getFPS() const { return mFPS; }

        const ThreadSafeGame & getThreadSafeGame() const;

        ThreadSafeGame & getThreadSafeGame();

        virtual bool initAttributeControllers();

        virtual int calculateWidth(XULWin::SizeConstraint inSizeConstraint) const;

        virtual int calculateHeight(XULWin::SizeConstraint inSizeConstraint) const;

        virtual LRESULT handleMessage(UINT inMessage, WPARAM wParam, LPARAM lParam);

        // NumFutureBlocksController
        virtual int getNumFutureBlocks() const;

        virtual void setNumFutureBlocks(int inNumFutureBlocks);

        virtual bool getKeyboardEnabled() const;

        virtual void setKeyboardEnabled(bool inKeyboardEnabled);

    private:
        LRESULT onKeyDown(WPARAM wParam, LPARAM lParam);

        static LRESULT CALLBACK KeyboardProc(int inCode, WPARAM wParam, LPARAM lParam);

        LRESULT keyboardProc(int inCode, WPARAM wParam, LPARAM lParam);

        void onTimerEvent();

        void bufferedPaint(HDC inHDC);

        virtual void paint(HDC inHDC);

        boost::scoped_ptr<ThreadSafeGame> mThreadSafeGame;
        int mNumFutureBlocks;
        bool mKeyboardEnabled;
        XULWin::WinAPI::Timer mWinAPITimer;
        int mFrameCount;
        int mFPS;
        Poco::Stopwatch mFPSStopwatch;
    };

} // namespace Tetris


#endif // TETRISCOMPONENT_H_INCLUDED
