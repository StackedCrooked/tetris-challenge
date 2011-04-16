QT += core gui
TARGET = QtTetris
TEMPLATE = app
CONFIG += i386


INCLUDEPATH += \
    . \
    Tetris/include \
    Futile/include \
    3rdParty/Boost_1_44_0 \
    3rdParty/Poco/Foundation/include


SOURCES += \
    QtTetris/main.cpp \
    QtTetris/MainWindow.cpp \
    QtTetris/Model.cpp \
    QtTetris/NewGameDialog.cpp \
    QtTetris/QtMainThread.cpp \
    QtTetris/TetrisWidget.cpp \
    Tetris/src/AbstractWidget.cpp \
    Tetris/src/AISupport.cpp \
    Tetris/src/Block.cpp \
    Tetris/src/BlockFactory.cpp \
    Tetris/src/BlockMover.cpp \
    Tetris/src/BlockType.cpp \
    Tetris/src/ComputerPlayer.cpp \
    Tetris/src/Evaluator.cpp \
    Tetris/src/EvilBlockFactory.cpp \
    Tetris/src/Game.cpp \
    Tetris/src/GameState.cpp \
    Tetris/src/GameStateComparator.cpp \
    Tetris/src/GameStateNode.cpp \
    Tetris/src/Gravity.cpp \
    Tetris/src/MultiplayerGame.cpp \
    Tetris/src/MultiThreadedNodeCalculator.cpp \
    Tetris/src/NodeCalculator.cpp \
    Tetris/src/NodeCalculatorImpl.cpp \
    Tetris/src/Player.cpp \
    Tetris/src/SimpleGame.cpp \
    Tetris/src/SingleThreadedNodeCalculator.cpp \
    Tetris/src/Unicode.cpp \
    Tetris/testsuite/src/Driver.cpp \
    Tetris/testsuite/src/GenericGridTest.cpp \
    Tetris/testsuite/src/NodeCalculatorTest.cpp \
    Tetris/testsuite/src/TetrisTestSuite.cpp \
    Tetris/testsuite/src/WorkerPoolTest.cpp \
    Tetris/testsuite/src/WorkerTest.cpp \
    Futile/src/Logger.cpp \
    Futile/src/Logging.cpp \
    Futile/src/MainThread.cpp \
    Futile/src/Threading.cpp \
    Futile/src/Worker.cpp \
    Futile/src/WorkerPool.cpp \
    Futile/src/LeakDetector.cpp


HEADERS += \
    QtTetris/MainWindow.h \
    QtTetris/Model.h \
    QtTetris/NewGameDialog.h \
    QtTetris/QtMainThread.h \
    QtTetris/TetrisWidget.h \
    Tetris/include/Tetris/AbstractWidget.h \
    Tetris/include/Tetris/AISupport.h \
    Tetris/include/Tetris/Block.h \
    Tetris/include/Tetris/BlockFactory.h \
    Tetris/include/Tetris/BlockMover.h \
    Tetris/include/Tetris/BlockType.h \
    Tetris/include/Tetris/BlockTypes.h \
    Tetris/include/Tetris/ComputerPlayer.h \
    Tetris/include/Tetris/Config.h \
    Tetris/include/Tetris/Direction.h \
    Tetris/include/Tetris/Evaluator.h \
    Tetris/include/Tetris/EvilBlockFactory.h \
    Tetris/include/Tetris/ForwardDeclarations.h \
    Tetris/include/Tetris/Game.h \
    Tetris/include/Tetris/GameOver.h \
    Tetris/include/Tetris/GameState.h \
    Tetris/include/Tetris/GameStateComparator.h \
    Tetris/include/Tetris/GameStateNode.h \
    Tetris/include/Tetris/GameStateStats.h \
    Tetris/include/Tetris/Gravity.h \
    Tetris/include/Tetris/Grid.h \
    Tetris/include/Tetris/MultiplayerGame.h \
    Tetris/include/Tetris/MultithreadedNodeCalculator.h \
    Tetris/include/Tetris/NodeCalculator.h \
    Tetris/include/Tetris/NodeCalculatorImpl.h \
    Tetris/include/Tetris/NodePtr.h \
    Tetris/include/Tetris/Player.h \
    Tetris/include/Tetris/PlayerType.h \
    Tetris/include/Tetris/SimpleGame.h \
    Tetris/include/Tetris/SingleThreadedNodeCalculator.h \
    Tetris/include/Tetris/Tetris.h \
    Tetris/include/Tetris/Unicode.h \
    Tetris/include/Tetris/Utilities.h \
    Tetris/testsuite/src/GenericGridTest.h \
    Tetris/testsuite/src/NodeCalculatorTest.h \
    Tetris/testsuite/src/TetrisTestSuite.h \
    Tetris/testsuite/src/WorkerPoolTest.h \
    Tetris/testsuite/src/WorkerTest.h \
    Futile/include/Futile/Allocator.h \
    Futile/include/Futile/Array.h \
    Futile/include/Futile/Assert.h \
    Futile/include/Futile/AutoPtrSupport.h \
    Futile/include/Futile/Boost.h \
    Futile/include/Futile/Config.h \
    Futile/include/Futile/GenericGrid.h \
    Futile/include/Futile/Logger.h \
    Futile/include/Futile/Logging.h \
    Futile/include/Futile/MainThread.h \
    Futile/include/Futile/MainThreadImpl.h \
    Futile/include/Futile/MakeString.h \
    Futile/include/Futile/Threading.h \
    Futile/include/Futile/TypedWrapper.h \
    Futile/include/Futile/Worker.h \
    Futile/include/Futile/WorkerPool.h \
    3rdParty/Poco/Foundation/include/Poco/AbstractCache.h \
    3rdParty/Poco/Foundation/include/Poco/AbstractDelegate.h \
    3rdParty/Poco/Foundation/include/Poco/AbstractEvent.h \
    3rdParty/Poco/Foundation/include/Poco/AbstractObserver.h \
    3rdParty/Poco/Foundation/include/Poco/AbstractPriorityDelegate.h \
    3rdParty/Poco/Foundation/include/Poco/AbstractStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/AccessExpirationDecorator.h \
    3rdParty/Poco/Foundation/include/Poco/AccessExpireCache.h \
    3rdParty/Poco/Foundation/include/Poco/AccessExpireLRUCache.h \
    3rdParty/Poco/Foundation/include/Poco/AccessExpireStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/ActiveDispatcher.h \
    3rdParty/Poco/Foundation/include/Poco/ActiveMethod.h \
    3rdParty/Poco/Foundation/include/Poco/ActiveResult.h \
    3rdParty/Poco/Foundation/include/Poco/ActiveRunnable.h \
    3rdParty/Poco/Foundation/include/Poco/ActiveStarter.h \
    3rdParty/Poco/Foundation/include/Poco/Activity.h \
    3rdParty/Poco/Foundation/include/Poco/Any.h \
    3rdParty/Poco/Foundation/include/Poco/ArchiveStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/ASCIIEncoding.h \
    3rdParty/Poco/Foundation/include/Poco/AsyncChannel.h \
    3rdParty/Poco/Foundation/include/Poco/AtomicCounter.h \
    3rdParty/Poco/Foundation/include/Poco/AutoPtr.h \
    3rdParty/Poco/Foundation/include/Poco/AutoReleasePool.h \
    3rdParty/Poco/Foundation/include/Poco/Base64Decoder.h \
    3rdParty/Poco/Foundation/include/Poco/Base64Encoder.h \
    3rdParty/Poco/Foundation/include/Poco/BasicEvent.h \
    3rdParty/Poco/Foundation/include/Poco/BinaryReader.h \
    3rdParty/Poco/Foundation/include/Poco/BinaryWriter.h \
    3rdParty/Poco/Foundation/include/Poco/Buffer.h \
    3rdParty/Poco/Foundation/include/Poco/BufferAllocator.h \
    3rdParty/Poco/Foundation/include/Poco/BufferedBidirectionalStreamBuf.h \
    3rdParty/Poco/Foundation/include/Poco/BufferedStreamBuf.h \
    3rdParty/Poco/Foundation/include/Poco/Bugcheck.h \
    3rdParty/Poco/Foundation/include/Poco/ByteOrder.h \
    3rdParty/Poco/Foundation/include/Poco/Channel.h \
    3rdParty/Poco/Foundation/include/Poco/Checksum.h \
    3rdParty/Poco/Foundation/include/Poco/ClassLibrary.h \
    3rdParty/Poco/Foundation/include/Poco/ClassLoader.h \
    3rdParty/Poco/Foundation/include/Poco/CompareFunctions.h \
    3rdParty/Poco/Foundation/include/Poco/Condition.h \
    3rdParty/Poco/Foundation/include/Poco/Config.h \
    3rdParty/Poco/Foundation/include/Poco/Configurable.h \
    3rdParty/Poco/Foundation/include/Poco/ConsoleChannel.h \
    3rdParty/Poco/Foundation/include/Poco/CountingStream.h \
    3rdParty/Poco/Foundation/include/Poco/DateTime.h \
    3rdParty/Poco/Foundation/include/Poco/DateTimeFormat.h \
    3rdParty/Poco/Foundation/include/Poco/DateTimeFormatter.h \
    3rdParty/Poco/Foundation/include/Poco/DateTimeParser.h \
    3rdParty/Poco/Foundation/include/Poco/Debugger.h \
    3rdParty/Poco/Foundation/include/Poco/DefaultStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/DeflatingStream.h \
    3rdParty/Poco/Foundation/include/Poco/Delegate.h \
    3rdParty/Poco/Foundation/include/Poco/DigestEngine.h \
    3rdParty/Poco/Foundation/include/Poco/DigestStream.h \
    3rdParty/Poco/Foundation/include/Poco/DirectoryIterator.h \
    3rdParty/Poco/Foundation/include/Poco/DirectoryIterator_UNIX.h \
    3rdParty/Poco/Foundation/include/Poco/DirectoryIterator_VMS.h \
    3rdParty/Poco/Foundation/include/Poco/DirectoryIterator_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/DirectoryIterator_WIN32U.h \
    3rdParty/Poco/Foundation/include/Poco/DynamicAny.h \
    3rdParty/Poco/Foundation/include/Poco/DynamicAnyHolder.h \
    3rdParty/Poco/Foundation/include/Poco/DynamicFactory.h \
    3rdParty/Poco/Foundation/include/Poco/Environment.h \
    3rdParty/Poco/Foundation/include/Poco/Environment_UNIX.h \
    3rdParty/Poco/Foundation/include/Poco/Environment_VMS.h \
    3rdParty/Poco/Foundation/include/Poco/Environment_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/Environment_WIN32U.h \
    3rdParty/Poco/Foundation/include/Poco/ErrorHandler.h \
    3rdParty/Poco/Foundation/include/Poco/Event.h \
    3rdParty/Poco/Foundation/include/Poco/Event_POSIX.h \
    3rdParty/Poco/Foundation/include/Poco/Event_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/EventArgs.h \
    3rdParty/Poco/Foundation/include/Poco/EventLogChannel.h \
    3rdParty/Poco/Foundation/include/Poco/Exception.h \
    3rdParty/Poco/Foundation/include/Poco/ExpirationDecorator.h \
    3rdParty/Poco/Foundation/include/Poco/Expire.h \
    3rdParty/Poco/Foundation/include/Poco/ExpireCache.h \
    3rdParty/Poco/Foundation/include/Poco/ExpireLRUCache.h \
    3rdParty/Poco/Foundation/include/Poco/ExpireStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/FIFOEvent.h \
    3rdParty/Poco/Foundation/include/Poco/FIFOStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/File.h \
    3rdParty/Poco/Foundation/include/Poco/File_UNIX.h \
    3rdParty/Poco/Foundation/include/Poco/File_VMS.h \
    3rdParty/Poco/Foundation/include/Poco/File_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/File_WIN32U.h \
    3rdParty/Poco/Foundation/include/Poco/FileChannel.h \
    3rdParty/Poco/Foundation/include/Poco/FileStream.h \
    3rdParty/Poco/Foundation/include/Poco/FileStream_POSIX.h \
    3rdParty/Poco/Foundation/include/Poco/FileStream_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/FileStreamFactory.h \
    3rdParty/Poco/Foundation/include/Poco/Format.h \
    3rdParty/Poco/Foundation/include/Poco/Formatter.h \
    3rdParty/Poco/Foundation/include/Poco/FormattingChannel.h \
    3rdParty/Poco/Foundation/include/Poco/Foundation.h \
    3rdParty/Poco/Foundation/include/Poco/FPEnvironment.h \
    3rdParty/Poco/Foundation/include/Poco/FPEnvironment_C99.h \
    3rdParty/Poco/Foundation/include/Poco/FPEnvironment_DEC.h \
    3rdParty/Poco/Foundation/include/Poco/FPEnvironment_DUMMY.h \
    3rdParty/Poco/Foundation/include/Poco/FPEnvironment_SUN.h \
    3rdParty/Poco/Foundation/include/Poco/FPEnvironment_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/FunctionDelegate.h \
    3rdParty/Poco/Foundation/include/Poco/FunctionPriorityDelegate.h \
    3rdParty/Poco/Foundation/include/Poco/Glob.h \
    3rdParty/Poco/Foundation/include/Poco/Hash.h \
    3rdParty/Poco/Foundation/include/Poco/HashFunction.h \
    3rdParty/Poco/Foundation/include/Poco/HashMap.h \
    3rdParty/Poco/Foundation/include/Poco/HashSet.h \
    3rdParty/Poco/Foundation/include/Poco/HashStatistic.h \
    3rdParty/Poco/Foundation/include/Poco/HashTable.h \
    3rdParty/Poco/Foundation/include/Poco/HexBinaryDecoder.h \
    3rdParty/Poco/Foundation/include/Poco/HexBinaryEncoder.h \
    3rdParty/Poco/Foundation/include/Poco/HMACEngine.h \
    3rdParty/Poco/Foundation/include/Poco/InflatingStream.h \
    3rdParty/Poco/Foundation/include/Poco/Instantiator.h \
    3rdParty/Poco/Foundation/include/Poco/KeyValueArgs.h \
    3rdParty/Poco/Foundation/include/Poco/Latin1Encoding.h \
    3rdParty/Poco/Foundation/include/Poco/Latin9Encoding.h \
    3rdParty/Poco/Foundation/include/Poco/LinearHashTable.h \
    3rdParty/Poco/Foundation/include/Poco/LineEndingConverter.h \
    3rdParty/Poco/Foundation/include/Poco/LocalDateTime.h \
    3rdParty/Poco/Foundation/include/Poco/LogFile.h \
    3rdParty/Poco/Foundation/include/Poco/LogFile_STD.h \
    3rdParty/Poco/Foundation/include/Poco/LogFile_VMS.h \
    3rdParty/Poco/Foundation/include/Poco/LogFile_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/LogFile_WIN32U.h \
    3rdParty/Poco/Foundation/include/Poco/Logger.h \
    3rdParty/Poco/Foundation/include/Poco/LoggingFactory.h \
    3rdParty/Poco/Foundation/include/Poco/LoggingRegistry.h \
    3rdParty/Poco/Foundation/include/Poco/LogStream.h \
    3rdParty/Poco/Foundation/include/Poco/LRUCache.h \
    3rdParty/Poco/Foundation/include/Poco/LRUStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/Manifest.h \
    3rdParty/Poco/Foundation/include/Poco/MD2Engine.h \
    3rdParty/Poco/Foundation/include/Poco/MD4Engine.h \
    3rdParty/Poco/Foundation/include/Poco/MD5Engine.h \
    3rdParty/Poco/Foundation/include/Poco/MemoryPool.h \
    3rdParty/Poco/Foundation/include/Poco/MemoryStream.h \
    3rdParty/Poco/Foundation/include/Poco/Message.h \
    3rdParty/Poco/Foundation/include/Poco/MetaObject.h \
    3rdParty/Poco/Foundation/include/Poco/MetaProgramming.h \
    3rdParty/Poco/Foundation/include/Poco/Mutex.h \
    3rdParty/Poco/Foundation/include/Poco/Mutex_POSIX.h \
    3rdParty/Poco/Foundation/include/Poco/Mutex_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/NamedEvent.h \
    3rdParty/Poco/Foundation/include/Poco/NamedEvent_UNIX.h \
    3rdParty/Poco/Foundation/include/Poco/NamedEvent_VMS.h \
    3rdParty/Poco/Foundation/include/Poco/NamedEvent_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/NamedEvent_WIN32U.h \
    3rdParty/Poco/Foundation/include/Poco/NamedMutex.h \
    3rdParty/Poco/Foundation/include/Poco/NamedMutex_UNIX.h \
    3rdParty/Poco/Foundation/include/Poco/NamedMutex_VMS.h \
    3rdParty/Poco/Foundation/include/Poco/NamedMutex_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/NamedMutex_WIN32U.h \
    3rdParty/Poco/Foundation/include/Poco/NamedTuple.h \
    3rdParty/Poco/Foundation/include/Poco/NestedDiagnosticContext.h \
    3rdParty/Poco/Foundation/include/Poco/NObserver.h \
    3rdParty/Poco/Foundation/include/Poco/Notification.h \
    3rdParty/Poco/Foundation/include/Poco/NotificationCenter.h \
    3rdParty/Poco/Foundation/include/Poco/NotificationQueue.h \
    3rdParty/Poco/Foundation/include/Poco/NotificationStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/NullChannel.h \
    3rdParty/Poco/Foundation/include/Poco/NullStream.h \
    3rdParty/Poco/Foundation/include/Poco/NumberFormatter.h \
    3rdParty/Poco/Foundation/include/Poco/NumberParser.h \
    3rdParty/Poco/Foundation/include/Poco/Observer.h \
    3rdParty/Poco/Foundation/include/Poco/OpcomChannel.h \
    3rdParty/Poco/Foundation/include/Poco/Path.h \
    3rdParty/Poco/Foundation/include/Poco/Path_UNIX.h \
    3rdParty/Poco/Foundation/include/Poco/Path_VMS.h \
    3rdParty/Poco/Foundation/include/Poco/Path_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/Path_WIN32U.h \
    3rdParty/Poco/Foundation/include/Poco/PatternFormatter.h \
    3rdParty/Poco/Foundation/include/Poco/Pipe.h \
    3rdParty/Poco/Foundation/include/Poco/PipeImpl.h \
    3rdParty/Poco/Foundation/include/Poco/PipeImpl_DUMMY.h \
    3rdParty/Poco/Foundation/include/Poco/PipeImpl_POSIX.h \
    3rdParty/Poco/Foundation/include/Poco/PipeImpl_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/PipeStream.h \
    3rdParty/Poco/Foundation/include/Poco/Platform.h \
    3rdParty/Poco/Foundation/include/Poco/Platform_POSIX.h \
    3rdParty/Poco/Foundation/include/Poco/Platform_VMS.h \
    3rdParty/Poco/Foundation/include/Poco/Platform_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/Poco.h \
    3rdParty/Poco/Foundation/include/Poco/PriorityDelegate.h \
    3rdParty/Poco/Foundation/include/Poco/PriorityEvent.h \
    3rdParty/Poco/Foundation/include/Poco/PriorityExpire.h \
    3rdParty/Poco/Foundation/include/Poco/PriorityNotificationQueue.h \
    3rdParty/Poco/Foundation/include/Poco/Process.h \
    3rdParty/Poco/Foundation/include/Poco/Process_UNIX.h \
    3rdParty/Poco/Foundation/include/Poco/Process_VMS.h \
    3rdParty/Poco/Foundation/include/Poco/Process_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/Process_WIN32U.h \
    3rdParty/Poco/Foundation/include/Poco/PurgeStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/Random.h \
    3rdParty/Poco/Foundation/include/Poco/RandomStream.h \
    3rdParty/Poco/Foundation/include/Poco/RefCountedObject.h \
    3rdParty/Poco/Foundation/include/Poco/RegularExpression.h \
    3rdParty/Poco/Foundation/include/Poco/RotateStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/Runnable.h \
    3rdParty/Poco/Foundation/include/Poco/RunnableAdapter.h \
    3rdParty/Poco/Foundation/include/Poco/RWLock.h \
    3rdParty/Poco/Foundation/include/Poco/RWLock_POSIX.h \
    3rdParty/Poco/Foundation/include/Poco/RWLock_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/ScopedLock.h \
    3rdParty/Poco/Foundation/include/Poco/ScopedUnlock.h \
    3rdParty/Poco/Foundation/include/Poco/Semaphore.h \
    3rdParty/Poco/Foundation/include/Poco/Semaphore_POSIX.h \
    3rdParty/Poco/Foundation/include/Poco/Semaphore_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/SHA1Engine.h \
    3rdParty/Poco/Foundation/include/Poco/SharedLibrary.h \
    3rdParty/Poco/Foundation/include/Poco/SharedLibrary_HPUX.h \
    3rdParty/Poco/Foundation/include/Poco/SharedLibrary_UNIX.h \
    3rdParty/Poco/Foundation/include/Poco/SharedLibrary_VMS.h \
    3rdParty/Poco/Foundation/include/Poco/SharedLibrary_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/SharedLibrary_WIN32U.h \
    3rdParty/Poco/Foundation/include/Poco/SharedMemory.h \
    3rdParty/Poco/Foundation/include/Poco/SharedMemory_DUMMY.h \
    3rdParty/Poco/Foundation/include/Poco/SharedMemory_POSIX.h \
    3rdParty/Poco/Foundation/include/Poco/SharedMemory_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/SharedPtr.h \
    3rdParty/Poco/Foundation/include/Poco/SignalHandler.h \
    3rdParty/Poco/Foundation/include/Poco/SimpleFileChannel.h \
    3rdParty/Poco/Foundation/include/Poco/SimpleHashTable.h \
    3rdParty/Poco/Foundation/include/Poco/SingletonHolder.h \
    3rdParty/Poco/Foundation/include/Poco/SplitterChannel.h \
    3rdParty/Poco/Foundation/include/Poco/Stopwatch.h \
    3rdParty/Poco/Foundation/include/Poco/StrategyCollection.h \
    3rdParty/Poco/Foundation/include/Poco/StreamChannel.h \
    3rdParty/Poco/Foundation/include/Poco/StreamConverter.h \
    3rdParty/Poco/Foundation/include/Poco/StreamCopier.h \
    3rdParty/Poco/Foundation/include/Poco/StreamTokenizer.h \
    3rdParty/Poco/Foundation/include/Poco/StreamUtil.h \
    3rdParty/Poco/Foundation/include/Poco/String.h \
    3rdParty/Poco/Foundation/include/Poco/StringTokenizer.h \
    3rdParty/Poco/Foundation/include/Poco/SynchronizedObject.h \
    3rdParty/Poco/Foundation/include/Poco/SyslogChannel.h \
    3rdParty/Poco/Foundation/include/Poco/Task.h \
    3rdParty/Poco/Foundation/include/Poco/TaskManager.h \
    3rdParty/Poco/Foundation/include/Poco/TaskNotification.h \
    3rdParty/Poco/Foundation/include/Poco/TeeStream.h \
    3rdParty/Poco/Foundation/include/Poco/TemporaryFile.h \
    3rdParty/Poco/Foundation/include/Poco/TextConverter.h \
    3rdParty/Poco/Foundation/include/Poco/TextEncoding.h \
    3rdParty/Poco/Foundation/include/Poco/TextIterator.h \
    3rdParty/Poco/Foundation/include/Poco/Thread.h \
    3rdParty/Poco/Foundation/include/Poco/Thread_POSIX.h \
    3rdParty/Poco/Foundation/include/Poco/Thread_WIN32.h \
    3rdParty/Poco/Foundation/include/Poco/ThreadLocal.h \
    3rdParty/Poco/Foundation/include/Poco/ThreadPool.h \
    3rdParty/Poco/Foundation/include/Poco/ThreadTarget.h \
    3rdParty/Poco/Foundation/include/Poco/TimedNotificationQueue.h \
    3rdParty/Poco/Foundation/include/Poco/Timer.h \
    3rdParty/Poco/Foundation/include/Poco/Timespan.h \
    3rdParty/Poco/Foundation/include/Poco/Timestamp.h \
    3rdParty/Poco/Foundation/include/Poco/Timezone.h \
    3rdParty/Poco/Foundation/include/Poco/Token.h \
    3rdParty/Poco/Foundation/include/Poco/Tuple.h \
    3rdParty/Poco/Foundation/include/Poco/TypeList.h \
    3rdParty/Poco/Foundation/include/Poco/Types.h \
    3rdParty/Poco/Foundation/include/Poco/UnbufferedStreamBuf.h \
    3rdParty/Poco/Foundation/include/Poco/Unicode.h \
    3rdParty/Poco/Foundation/include/Poco/UnicodeConverter.h \
    3rdParty/Poco/Foundation/include/Poco/UniqueAccessExpireCache.h \
    3rdParty/Poco/Foundation/include/Poco/UniqueAccessExpireLRUCache.h \
    3rdParty/Poco/Foundation/include/Poco/UniqueAccessExpireStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/UniqueExpireCache.h \
    3rdParty/Poco/Foundation/include/Poco/UniqueExpireLRUCache.h \
    3rdParty/Poco/Foundation/include/Poco/UniqueExpireStrategy.h \
    3rdParty/Poco/Foundation/include/Poco/UnWindows.h \
    3rdParty/Poco/Foundation/include/Poco/URI.h \
    3rdParty/Poco/Foundation/include/Poco/URIStreamFactory.h \
    3rdParty/Poco/Foundation/include/Poco/URIStreamOpener.h \
    3rdParty/Poco/Foundation/include/Poco/UTF16Encoding.h \
    3rdParty/Poco/Foundation/include/Poco/UTF8Encoding.h \
    3rdParty/Poco/Foundation/include/Poco/UTF8String.h \
    3rdParty/Poco/Foundation/include/Poco/UUID.h \
    3rdParty/Poco/Foundation/include/Poco/UUIDGenerator.h \
    3rdParty/Poco/Foundation/include/Poco/ValidArgs.h \
    3rdParty/Poco/Foundation/include/Poco/Void.h \
    3rdParty/Poco/Foundation/include/Poco/Windows1252Encoding.h \
    3rdParty/Poco/Foundation/include/Poco/WindowsConsoleChannel.h \
    3rdParty/Poco/Foundation/include/Poco/zconf.h \
    3rdParty/Poco/Foundation/include/Poco/zlib.h \
    3rdParty/Poco/Foundation/include/pocomsg.h \
    Futile/include/Futile/LeakDetector.h \
    Futile/include/Futile/Singleton.h


OTHER_FILES += \
    Futile/CMakeLists.txt \
    QtTetris/CMakeLists.txt \
    Tetris/CMakeLists.txt \
    Tetris/testsuite/CMakeLists.txt
