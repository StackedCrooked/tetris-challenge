project "Foundation"
  kind "StaticLib"
  targetdir "../lib"
  
  language "c"
  
  configuration "Debug" 
    targetname "PocoFoundationmtd"
  configuration "Release"
    targetname "PocoFoundationmt"

  configuration { }

  defines { 
           "WIN32",
           "_WINDOWS",
           "Foundation_EXPORTS",
           "POCO_STATIC",
           "PCRE_STATIC",
           "WINVER=0x0500",
           "POCO_NO_FPENVIRONMENT",
           "MINGW32",
           "POCO_THREAD_STACK_SIZE",
           "Foundation_Config_INCLUDED"
          }

  configuration "Debug"
    defines { "_DEBUG" }

  configuration { } 

  includedirs {  
                "include"
              }

  linkoptions { 
                "-Wl,--allow-multiple-definition" 
              }

  files { 
           "include/**.h",
           "src/*.cpp",
           "src/*.c"
        }
-- Exclude all c++ files which are included in the main c++ files                
  excludes {
             "src/DirectoryIterator_UNIX.cpp",
             "src/DirectoryIterator_VMS.cpp",
             "src/DirectoryIterator_WIN32U.cpp",
             "src/DirectoryIterator_WIN32.cpp",
             "src/Environment_UNIX.cpp",
             "src/Environment_VMS.cpp",
             "src/Environment_WIN32.cpp",
             "src/Environment_WIN32U.cpp",
             "src/FPEnvironment.cpp",
             "src/FPEnvironment_C99.cpp",
             "src/FPEnvironment_DEC.cpp",
             "src/FPEnvironment_DUMMY.cpp",
             "src/FPEnvironment_SUN.cpp",
             "src/FPEnvironment_WIN32.cpp",
             "src/File_UNIX.cpp",
             "src/File_VMS.cpp",
             "src/File_WIN32.cpp",
             "src/File_WIN32U.cpp",
             "src/Path_UNIX.cpp",
             "src/Path_VMS.cpp",
             "src/Path_WIN32U.cpp",
             "src/Path_WIN32.cpp",
             "src/LogFile_STD.cpp",
             "src/LogFile_VMS.cpp",
             "src/LogFile_WIN32.cpp",
             "src/LogFile_WIN32U.cpp",
             "src/OpcomChannel.cpp",
             "src/SyslogChannel.cpp",
             "src/Timezone_UNIX.cpp",
             "src/Timezone_WIN32.cpp",
             "src/NamedEvent_UNIX.cpp",
             "src/NamedEvent_VMS.cpp",
             "src/NamedEvent_WIN32.cpp",
             "src/NamedEvent_WIN32U.cpp",
             "src/NamedMutex_UNIX.cpp",
             "src/NamedMutex_VMS.cpp",
             "src/NamedMutex_WIN32.cpp",
             "src/NamedMutex_WIN32U.cpp",
             "src/PipeImpl_DUMMY.cpp",
             "src/PipeImpl_POSIX.cpp",
             "src/PipeImpl_WIN32.cpp",
             "src/Process_UNIX.cpp",
             "src/Process_VMS.cpp",
             "src/Process_WIN32.cpp",
             "src/Process_WIN32U.cpp",
             "src/SharedMemory_DUMMY.cpp",
             "src/SharedMemory_POSIX.cpp",
             "src/SharedMemory_WIN32.cpp",
             "src/SharedLibrary_HPUX.cpp",
             "src/SharedLibrary_UNIX.cpp",
             "src/SharedLibrary_VMS.cpp",
             "src/SharedLibrary_WIN32.cpp",
             "src/SharedLibrary_WIN32U.cpp",
             "src/FileStream_POSIX.cpp",
             "src/FileStream_WIN32.cpp",
             "src/Event_POSIX.cpp",
             "src/Event_WIN32.cpp",
             "src/Mutex_POSIX.cpp",
             "src/Mutex_WIN32.cpp",
             "src/RWLock_POSIX.cpp",
             "src/RWLock_WIN32.cpp",
             "src/Semaphore_POSIX.cpp",
             "src/Semaphore_WIN32.cpp",
             "src/Thread_POSIX.cpp",
             "src/Thread_WIN32.cpp"
           }
