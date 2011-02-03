#ifndef QTMAINTHREAD_H
#define QTMAINTHREAD_H


#include "Tetris/MainThreadImpl.h"
#include <QtCore>


namespace Tetris {


class QtMainThread : public QObject,
                     public MainThreadImpl
{
public:
    QtMainThread();

    virtual ~QtMainThread();

    virtual void postAction(Action inAction);

    virtual bool event(QEvent * inEvent);
};


} // namespace Tetris


#endif // QTMAINTHREAD_H
