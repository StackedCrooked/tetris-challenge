#include "QtMainThread.h"
#include "Futile/AutoPtrSupport.h"
#include "Futile/Logging.h"


using Futile::Action;
using Futile::MainThreadImpl;


namespace Futile {


// Used in MainThread.cpp.
std::auto_ptr<MainThreadImpl> CreateMainThreadImpl()
{
    std::auto_ptr<MainThreadImpl> result;
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
        try
        {
            mAction();
        }
        catch (const std::exception& exc)
        {
            Futile::LogError(std::string("QtAction::invokeAction(): caught exception: ") + exc.what());
        }
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


bool QtMainThread::event(QEvent * inEvent)
{
    if (QtAction * action = dynamic_cast<QtAction*>(inEvent))
    {
        action->invokeAction();
        return true;
    }
    return QObject::event(inEvent);
}


} // namespace Tetris
