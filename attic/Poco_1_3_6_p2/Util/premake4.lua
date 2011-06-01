project "Util"
  kind "StaticLib"
  targetdir "../lib"
  language "c++"

  configuration "Debug" 
    targetname "PocoUtilmtd"
  configuration "Release"
    targetname "PocoUtilmt"

  configuration { } 
  
  defines { 
             "WIN32",
             "_WINDOWS",
             "Util_EXPORTS",
             "POCO_STATIC",
             "WINVER=0x0500",
             "POCO_NO_FPENVIRONMENT"
           }

  configuration "Debug"
    defines { "_DEBUG" }

  configuration { } 

  includedirs {  
                  "include"
                , "../Foundation/include"
                , "../XML/include"
              }

  files { 
          "include/**.h",
          "src/*.cpp"
        }
