#ifndef VISUALIZER_H_INCLUDED
#define VISUALIZER_H_INCLUDED


#include "BlockType.h"
#include "GameController.h"
#include "Grid.h"
#include <boost/noncopyable.hpp>
#include <map>
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

    class Visualizer : boost::noncopyable
    {
    public:
        Visualizer(GameController * inGameController);

        ~Visualizer();

        void show();

        void refresh();

        HWND handle();

    private:
        static void Initialize();
        static void Finalize();
        static LRESULT CALLBACK MessageHandler(HWND hWnd, UINT inMessage, WPARAM wParam, LPARAM lParam);

        static void CALLBACK TimerCallback(HWND hWnd, UINT inMessage, UINT_PTR inTimerID, DWORD inTime);

        void timerCallback();
        
        void paint(HDC inHDC);
        void bufferedPaint(HDC inHDC);
        void paintGrid(Gdiplus::Graphics & g);
        void paintUnit(Gdiplus::Graphics & g, int x, int y, BlockType inBlockType);

        static const Gdiplus::Color & GetColor(BlockType inType);

        HWND mHandle;
        HWND mUpButton;
        HWND mDownButton;
        typedef std::map<HWND, Visualizer *> Instances;

        GameController * mGameController;
        UINT_PTR mTimerID;
        int mDelay;

        static Instances sInstances;
        static ULONG_PTR sGdiPlusToken;
        static int sRefCount;
    };

} // namespace Tetris


#endif // VISUALIZER_H_INCLUDED
