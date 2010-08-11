#ifndef TETRISPAINTFUNCTIONS_H_INCLUDED
#define TETRISPAINTFUNCTIONS_H_INCLUDED


#include "GameState.h"
#include "BlockType.h"
#include "XULWin/Rect.h"
#include <string>


namespace Gdiplus
{
    class Brush;
    class Color;
    class Graphics;
}


namespace Tetris
{
    using XULWin::Rect;

    enum Coords
    {
        cUnitWidth    =               20,
        cUnitHeight   =               20
        //cWindowWidth  =               30 * cUnitHeight,
        //cWindowHeight =               30 * cUnitWidth,        
        //cFieldX       =                2 * cUnitWidth,
        //cFieldY       =                2 * cUnitHeight,
        //cNextBlocksX  =               14 * cUnitWidth,
        //cNextBlocksY  =                4 * cUnitHeight,
        //cScoresX      = cNextBlocksX + 6 * cUnitWidth,
        //cScoresY_0    =                4 * cUnitHeight,
        //cScoresY_1    =   cScoresY_0 + 4 * cUnitHeight,
        //cScoresY_2    =   cScoresY_1 + 4 * cUnitHeight,
        //cScoresY_3    =   cScoresY_2 + 4 * cUnitHeight,
        //cScoresY_4    =   cScoresY_3 + 4 * cUnitHeight,
        //cFPS          =   cScoresY_4 + 4 * cUnitHeight,
        //cScoresWidth  =                6 * cUnitWidth,
        //cScoresHeight =                6 * cUnitHeight
    };

    const Gdiplus::Color & GetColor(BlockType inType);

    void PaintUnit(Gdiplus::Graphics & g, int x, int y, BlockType inBlockType);

    void PaintGrid(Gdiplus::Graphics & g, const Grid & inGrid, int x, int y, bool inPaintBackground);
    
    //void PaintFutureBlocks(Gdiplus::Graphics & g, const BlockTypes & inBlockTypes);

    //void PaintText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Rect & inRect, const Gdiplus::Brush & inBrush);

    //void PaintText(Gdiplus::Graphics & inGraphics, const std::wstring & inText, const Rect & inRect);
    
    //void PaintScores(Gdiplus::Graphics & g, GameState);


} // namespace Tetris


#endif // TETRISPAINTFUNCTIONS_H_INCLUDED
