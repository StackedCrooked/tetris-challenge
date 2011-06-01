project "XML"
  kind "StaticLib"
  language "c++"
  targetdir "../lib"

  files { 
          "include/**.h",
          "src/*.cpp",
          "src/*.c"
        }

  excludes {
             "src/xmltok_ns.c",
             "src/xmltok_impl.c"
           }

  configuration "Debug" 
    targetname "PocoXMLmtd"
  configuration "Release"
    targetname "PocoXMLmt"

  configuration { }
  
  defines { 
           "WIN32",
           "_WINDOWS",
           "_USRDLL",
           "POCO_STATIC",
           "XML_EXPORTS",
           "XML_STATIC",
           "XML_NS",
           "XML_DTD",
           "HAVE_EXPAT_CONFIG_H",
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
