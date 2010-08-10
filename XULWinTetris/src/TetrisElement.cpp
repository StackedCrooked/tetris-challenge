#include "TetrisElement.h"


namespace Tetris
{

    XULWin::Component * CreateTetrisComponent(XULWin::Component *, const XULWin::AttributesMapping&);

    XULWin::ElementPtr TetrisElement::Create(XULWin::Element * inParent, const XULWin::AttributesMapping & inAttr)
    {
        return XULWin::Element::Create<TetrisElement>(inParent, inAttr);
    }

    TetrisElement::TetrisElement(XULWin::Element * inParent, const XULWin::AttributesMapping & inAttr) :
        XULWin::Element(TetrisElement::TagName(),
                inParent,
                CreateTetrisComponent(inParent->component(), inAttr))
    {
    }

    



} // namespace Tetris
