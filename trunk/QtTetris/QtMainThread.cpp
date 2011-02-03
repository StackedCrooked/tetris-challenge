#include "QtMainThread.h"
#include "Tetris/AutoPtrSupport.h"


namespace Tetris {


class QtAction : public QEvent
{
public:
    QtAction(Action inAction);

    void invokeAction();

private:
    Action mAction;
};


QtAction::QtAction(Action inAction) :
    QEvent(static_cast<QEvent::Type>(QEvent::User + 3210)),
    mAction(inAction)
{
}


void QtAction::invokeAction()
{
    if (mAction)
    {
        mAction();
    }
}


// Used in MainThread.cpp.
std::auto_ptr<MainThreadImpl> CreateMainThreadImpl()
{
    std::auto_ptr<MainThreadImpl> result;
    result.reset(new QtMainThread);
    return result;
}


QtMainThread::QtMainThread()
{
}


QtMainThread::~QtMainThread()
{
}


void QtMainThread::postAction(Action inAction)
{
    qApp->postEvent(this, new QtAction(inAction));
}


bool QtMainThread::event(QEvent * inEvent)
{
    if (QtAction * Action = dynamic_cast<QtAction*>(inEvent))
    {
        Action->invokeAction();
        return true;
    }
    return QObject::event(inEvent);
}


} // namespace Tetris
