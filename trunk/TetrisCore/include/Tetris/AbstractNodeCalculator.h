#ifndef TETRIS_ABSTRACTNODECALCULATOR_H_INCLUDED
#define TETRIS_ABSTRACTNODECALCULATOR_H_INCLUDED


#include "Tetris/NodePtr.h"


namespace Tetris
{


    class AbstractNodeCalculator
    {
    public:
        AbstractNodeCalculator() {}

        virtual ~AbstractNodeCalculator() = 0 {}

        virtual void start() = 0;

        virtual void stop() = 0;

        virtual int getCurrentSearchDepth() const = 0;

        virtual int getMaxSearchDepth() const = 0;

        enum Status
        {
            Status_Begin,
            Status_Nil = Status_Begin,
            Status_Started,
            Status_Working,
            Status_Stopped,
            Status_Finished,
            Status_End
        };
        
        virtual Status status() const = 0;

        virtual NodePtr result() const = 0;

    private:
        AbstractNodeCalculator(const AbstractNodeCalculator &);
        AbstractNodeCalculator & operator=(const AbstractNodeCalculator &);
    };

} // namespace Tetris


#endif // TETRIS_ABSTRACTNODECALCULATOR_H_INCLUDED
