solution "Hecato"
   configurations { "Debug", "Release" }
 
   -- A project defines one build target
defines { "LINUX" }


includedirs { "../include",
			  "../src/blobslib",
			  "../src/network",
			  "../external/TUIO-1.4",
			  "../external/TUIO-1.4/oscpack",
			  "/usr/include/opencv2",
			  "/usr/include/ni" }
 
libdirs { "../external/TUIO-1.4/" }

project "libHecato"
  targetdir "../lib/"
  kind "StaticLib"
  language "C++"
  files { "../include/**.h",
	      "../src/*.cpp",
	      "../src/*.h",
		  "../src/blobslib/*.h",
		  "../src/blobslib/*.cpp",
		  "../src/network/*.h",
		  "../src/network/*.cpp"}

configuration "Debug"
  defines {"LINUX", "DEBUG", "_DEBUG" }
  flags { "Symbols"}
  buildoptions { "-ggdb", "-fPIC" }
  links { "OpenNI" }
  objdir "../obj"
  targetname "HecatoD"
 
configuration "Release"
  defines {"LINUX", "NDEBUG" }
  flags { "Optimize"}
  buildoptions { "-fPIC" }
  links { "OpenNI" }
  objdir "../obj"
  targetname "Hecato"

project "HTCalibration"
  targetdir "../bin/"
  kind "WindowedApp"
  language "C++"
  files { "../src/HTCalibration/*.cpp",
		}

configuration "Debug"
  defines {"LINUX", "DEBUG", "_DEBUG" }
  flags { "Symbols"}
  buildoptions { "-ggdb"}
  links {"libHecato", "tinyxml", "opencv_core", "opencv_imgproc", "opencv_highgui", "opencv_features2d", "opencv_contrib", "opencv_objdetect", "opencv_video", "pthread", "OpenNI"}
  objdir "../obj"
  targetname "HTCalibrationD"
 
configuration "Release"
  defines {"LINUX", "NDEBUG" }
  flags { "Optimize"}
  buildoptions { "-fPIC" }
  links {"libHecato", "tinyxml", "opencv_core", "opencv_imgproc", "opencv_highgui", "opencv_features2d", "opencv_contrib", "opencv_objdetect", "opencv_video", "pthread", "OpenNI"}
  objdir "../obj"
  targetname "HTCalibration"

project "HTApp"
  targetdir "../bin/"
  kind "WindowedApp"
  language "C++"
  files { "../src/HTApp/*.cpp",
		}

configuration "Debug"
  defines {"LINUX", "DEBUG", "_DEBUG" }
  flags { "Symbols"}
  buildoptions { "-ggdb"}
  links {"libHecato", "tinyxml", "opencv_core", "opencv_imgproc", "opencv_highgui", "opencv_features2d", "opencv_contrib", "opencv_objdetect", "opencv_video", "pthread", "OpenNI", "TUIO",  "sfml-graphics", "sfml-window", "sfml-system"}
  objdir "../obj"
  targetname "HTAppD"
 
configuration "Release"
  defines {"LINUX", "NDEBUG" }
  flags { "Optimize"}
  buildoptions { "-fPIC" }
  links {"libHecato", "tinyxml", "opencv_core", "opencv_imgproc", "opencv_highgui", "opencv_features2d", "opencv_contrib", "opencv_objdetect", "opencv_video", "pthread", "OpenNI", "TUIO", "sfml-graphics", "sfml-window", "sfml-system"}
  objdir "../obj"
  targetname "HTApp"



