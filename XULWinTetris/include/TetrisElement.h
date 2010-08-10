#ifndef TETRISELEMENT_H_INCLUDED
#define TETRISELEMENT_H_INCLUDED


#include "XULWin/Element.h"


namespace Tetris
{

    class TetrisComponent;

    /**
     * You must initialize this class before attempting to parse a XUL document:
     *
     *   #include "XULWin/ElementFactory.h"
     *   ...
     *   XULWin::ElementFactory::Instance().registerElement<TetrisElement>();
     */
    class TetrisElement : public XULWin::Element
    {
    public:
        typedef TetrisComponent ComponentType;

        static XULWin::ElementPtr Create(XULWin::Element *, const XULWin::AttributesMapping &);

        static const char * TagName() { return "tetris"; }

    private:
        TetrisElement(XULWin::Element *, const XULWin::AttributesMapping &);

        friend class XULWin::Element;
    };

    
} // namespace Tetris


#endif // TETRISELEMENT_H_INCLUDED
