//#include "Tetris/MainThread.h"
//#include "Tetris/Logging.h"


//namespace Tetris {


//class ScheduledEvent : public QEvent
//{
//public:
//    ScheduledEvent(ScheduledAction * inAction) :
//        QEvent(QEvent::User + 3210), // how to ensure non-conflict here??
//        mAction(inAction)
//    {
//    }

//    virtual ~ScheduledEvent()
//    {
//    }

//    void invokeAction()
//    {
//        if (mAction)
//        {
//            (*mAction)();
//        }
//    }

//private:
//    boost::scoped_ptr<ScheduledAction> mAction;
//};


//MainThreadImpl::MainThreadImpl() :
//    mWidget(new QWidget)
//{

//}


//MainThreadImpl::~MainThreadImpl()
//{
//}


//void MainThreadImpl::post(Action * inAction)
//{
//    qApp->postEvent(this, new ScheduledEvent(inAction));
//}


//void MainThreadImpl::event(QEvent * inEvent)
//{
//    if (ScheduledEvent * event = dynamic_cast<ScheduledEvent*>(inEvent))
//    {
//        LogInfo("Received event. Let's invoke it!");
//        event->invokeAction();
//    }
//}


//} // namespace Tetris
