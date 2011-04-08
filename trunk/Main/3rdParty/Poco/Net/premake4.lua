project "Net"
  kind "StaticLib"
  targetdir "../lib"
  language "c++"
  
  configuration "Debug" 
    targetname "PocoNetmtd"
  configuration "Release"
    targetname "PocoNetmt"

  configuration { } 
  defines { 
             "WIN32",
             "_WINDOWS",
             "_USRDLL",
             "POCO_STATIC",
             "Net_EXPORTS", 
             "WINVER=0x0500",
             "POCO_NO_FPENVIRONMENT"
          }

  configuration "Debug"
    defines { "_DEBUG" }

  configuration { } 

  includedirs {  
                  "include"
                , "../Foundation/include"
              }

  files { 
          "include/**.h",
          "src/*.cpp"
        }
