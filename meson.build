project(
    'QtTetris',
    'cpp',
    default_options: [ 'cpp_std=c++11' ]
)


inc = include_directories(
    '3rdParty/gtest/include',
    'Futile/include',
    'Tetris/include',
    'QtTetris'
)


cxx = meson.get_compiler('cpp')

boost_dep = [
    cxx.find_library('boost_system'),
    cxx.find_library('boost_thread'),
]


poco_dep = [
    cxx.find_library('PocoFoundation')
]


qt5 = import('qt5')
qt5_dep = dependency(
    'qt5',
    modules: ['Core', 'Gui', 'Widgets']
)

moc_files = qt5.preprocess(
    moc_headers: [
        'QtTetris/MainWindow.h',
        'QtTetris/NewGameDialog.h',
        'QtTetris/TetrisWidget.h'
    ],
    include_directories: inc
)


executable('QtTetris',
    'Futile/src/LeakDetector.cpp',
    'Futile/src/Logger.cpp',
    'Futile/src/Logging.cpp',
    'Futile/src/MainThread.cpp',
    'Futile/src/Threading.cpp',
    'Futile/src/Worker.cpp',
    'Futile/src/WorkerPool.cpp',
    'QtTetris/main.cpp',
    'QtTetris/MainWindow.cpp',
    'QtTetris/Model.cpp',
    'QtTetris/NewGameDialog.cpp',
    'QtTetris/QtMainThread.cpp',
    'QtTetris/TetrisWidget.cpp',
    'Tetris/src/AbstractWidget.cpp',
    'Tetris/src/AISupport.cpp',
    'Tetris/src/Block.cpp',
    'Tetris/src/BlockFactory.cpp',
    'Tetris/src/BlockMover.cpp',
    'Tetris/src/BlockType.cpp',
    'Tetris/src/ComputerPlayer.cpp',
    'Tetris/src/Evaluator.cpp',
    'Tetris/src/Game.cpp',
    'Tetris/src/GameImpl.cpp',
    'Tetris/src/GameState.cpp',
    'Tetris/src/GameStateComparator.cpp',
    'Tetris/src/GameStateNode.cpp',
    'Tetris/src/Gravity.cpp',
    'Tetris/src/MultiplayerGame.cpp',
    'Tetris/src/MultiThreadedNodeCalculator.cpp',
    'Tetris/src/NodeCalculator.cpp',
    'Tetris/src/NodeCalculatorImpl.cpp',
    'Tetris/src/Player.cpp',
    'Tetris/src/SingleThreadedNodeCalculator.cpp',
    'Tetris/src/Unicode.cpp',
    moc_files,
    include_directories : inc,
    dependencies: [
        qt5_dep,
        poco_dep,
        boost_dep
    ]
)
