#ifndef QTMAINTHREAD_H
#define QTMAINTHREAD_H


#define QT_NO_KEYWORDS


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
