#include "QtMainThread.h"
#include "Futile/AutoPtrSupport.h"


using Futile::Action;
using Futile::MainThreadImpl;


namespace Futile {


// Used in MainThread.cpp.
std::unique_ptr<MainThreadImpl> CreateMainThreadImpl()
{
    std::unique_ptr<MainThreadImpl> result;
    result.reset(new Tetris::QtMainThread);
    return result;
}


} // namespace Futile


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


bool QtMainThread::event(QEvent* inEvent)
{
    if (QtAction * Action = dynamic_cast<QtAction*>(inEvent))
    {
        Action->invokeAction();
        return true;
    }
    return QObject::event(inEvent);
}


} // namespace Tetris
