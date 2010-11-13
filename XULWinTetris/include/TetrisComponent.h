#ifndef TETRISCOMPONENT_H_INCLUDED
#define TETRISCOMPONENT_H_INCLUDED


#include "Tetris/BlockType.h"
#include "Tetris/BlockTypes.h"
#include "Tetris/Direction.h"
#include "Tetris/Grid.h"
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
#include <boost/thread.hpp>


namespace Tetris
{
    class Block;
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
     * Attribute: numcolumns
     * Type: string (representing an integer)
     * Purpose: The number of future blocks to show in the Tetris component
     */
    class NumColumnsController : public XULWin::AttributeController
    {
    public:
        static const char * AttributeName()
        {
            return "numcolumns";
        }

        virtual void get(std::string & outValue);

        virtual void set(const std::string & inValue);

        virtual int getNumColumns() const = 0;

        virtual void setNumColumns(int inNumColumns) = 0;
    };


    /**
     * Attribute: numrows
     * Type: string (representing an integer)
     * Purpose: The number of future blocks to show in the Tetris component
     */
    class NumRowsController : public XULWin::AttributeController
    {
    public:
        static const char * AttributeName()
        {
            return "numrows";
        }

        virtual void get(std::string & outValue);

        virtual void set(const std::string & inValue);

        virtual int getNumRows() const = 0;

        virtual void setNumRows(int inNumRows) = 0;
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
                            public NumRowsController,
                            public NumColumnsController,
                            public KeyboardEnabledController,
                            public XULWin::GdiplusLoader
    {
    public:
        typedef XULWin::NativeControl Super;

        class Controller
        {
        public:
            virtual void getGameState(TetrisComponent * tetrisComponent, Grid & grid, Block & activeBlock, BlockTypes & futureBlockTypes) = 0;

            virtual bool move(TetrisComponent * tetrisComponent, Direction inDirection) = 0;

            virtual bool rotate(TetrisComponent * tetrisComponent) = 0;

            virtual void drop(TetrisComponent * tetrisComponent) = 0;
        };

        TetrisComponent(XULWin::Component * inParent, const XULWin::AttributesMapping & inAttr);

        virtual ~TetrisComponent();

        virtual bool init();

        void setController(Controller * inController);

        inline int getFPS() const { return mFPS; }

        virtual bool initAttributeControllers();

        virtual int calculateWidth(XULWin::SizeConstraint inSizeConstraint) const;

        virtual int calculateHeight(XULWin::SizeConstraint inSizeConstraint) const;

        virtual LRESULT handleMessage(UINT inMessage, WPARAM wParam, LPARAM lParam);

        // NumFutureBlocksController
        virtual int getNumFutureBlocks() const;

        virtual void setNumFutureBlocks(int inNumFutureBlocks);

        // NumColumnsController
        virtual int getNumColumns() const;

        virtual void setNumColumns(int inNumColumns);

        // NumRowsController
        virtual int getNumRows() const;

        virtual void setNumRows(int inNumRows);

        virtual bool getKeyboardEnabled() const;

        virtual void setKeyboardEnabled(bool inKeyboardEnabled);

    private:
        LRESULT onKeyDown(WPARAM wParam, LPARAM lParam);

        static LRESULT CALLBACK KeyboardProc(int inCode, WPARAM wParam, LPARAM lParam);

        LRESULT keyboardProc(int inCode, WPARAM wParam, LPARAM lParam);

        void onTimerEvent();

        void bufferedPaint(HDC inHDC);

        virtual void paint(HDC inHDC);

        Controller * mController;
        int mNumFutureBlocks;
        int mNumColumns;
        int mNumRows;
        bool mKeyboardEnabled;
        XULWin::WinAPI::Timer mWinAPITimer;
        int mFrameCount;
        int mFPS;
        Poco::Stopwatch mFPSStopwatch;
    };

} // namespace Tetris


#endif // TETRISCOMPONENT_H_INCLUDED
