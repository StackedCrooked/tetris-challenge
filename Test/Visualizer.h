#ifndef VISUALIZER_H_INCLUDED
#define VISUALIZER_H_INCLUDED


#include "BlockType.h"
#include "Game.h"
#include "Grid.h"
#include <boost/noncopyable.hpp>
#include <ctime>
#include <map>
#include <list>
#include <string>
#include <windows.h>


namespace Gdiplus
{
    class Brush;
    class Color;
    class Graphics;
    class RectF;
}


namespace Tetris
{

    class GameState;

    class Rect
    {
    public:
        Rect(int x, int y, int w, int h);

        int x() const;

        int y() const;

        int width() const;

        int height() const;

    private:
        int mX, mY, mWidth, mHeight;
    };

    class Visualizer : boost::noncopyable
    {
    public:
        Visualizer(Game * inGame);

        ~Visualizer();

        void show();

        void refresh();

        HWND handle();

    private:
        static void Initialize();
        static void Finalize();
        static LRESULT CALLBACK KeyboardProc(int inCode, WPARAM wParam, LPARAM lParam);
        static LRESULT CALLBACK MessageHandler(HWND hWnd, UINT inMessage, WPARAM wParam, LPARAM lParam);

        static void CALLBACK TimerCallback(HWND hWnd, UINT inMessage, UINT_PTR inTimerID, DWORD inTime);

        void timerCallback();
        void newComputerMove();
        void nextComputerMove();
        
        void paint(HDC inHDC);
        void bufferedPaint(HDC inHDC);
        void drawText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Rect & inRect);
        void drawText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Rect & inRect, const Gdiplus::Brush & inBrush);
        void paintGrid(Gdiplus::Graphics & g, const Grid & inGrid, int x, int y);
        void paintGrid(Gdiplus::Graphics & g);
        void paintScores(Gdiplus::Graphics & g);
        void paintFutureBlocks(Gdiplus::Graphics & g);
        void paintUnit(Gdiplus::Graphics & g, int x, int y, BlockType inBlockType);

        static const Gdiplus::Color & GetColor(BlockType inType);


        void appendKey(unsigned int inKeyCode);
        void processKeys();
        void processKey(unsigned int inKeyCode);
        std::vector<unsigned int> mKeys;

        HWND mHandle;
        HHOOK mKeyboardHook;

        Game * mGame;
        UINT_PTR mTimerID;
        int mDelay;
        clock_t mElapsed;
        clock_t mLastComputerMove;


        typedef std::map<HWND, Visualizer *> Instances;
        static Instances sInstances;
        static ULONG_PTR sGdiPlusToken;
        static int sRefCount;
    };

} // namespace Tetris


#endif // VISUALIZER_H_INCLUDED
