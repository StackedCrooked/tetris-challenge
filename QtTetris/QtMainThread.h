#ifndef QTMAINTHREAD_H
#define QTMAINTHREAD_H


#define QT_NO_KEYWORDS


// INFO: Fixes Clang build error. Maybe this can be removed in the future.
#ifndef QT_NO_STL
#define QT_NO_STL
#endif


#include "Futile/MainThreadImpl.h"
#include <QtCore/QtCore>


namespace Tetris {


class QtMainThread : public QObject,
                     public Futile::MainThreadImpl
{
public:
    QtMainThread();

    virtual ~QtMainThread();

    virtual void postAction(Futile::Action inAction);

    virtual bool event(QEvent * inEvent);
};


} // namespace Tetris


#endif // QTMAINTHREAD_H
