#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "07";
	static const char MONTH[] = "12";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.12";
	
	//Software Status
	static const char STATUS[] = "Beta";
	static const char STATUS_SHORT[] = "b";
	
	//Standard Version Type
	static const long MAJOR = 0;
	static const long MINOR = 5;
	static const long BUILD = 31;
	static const long REVISION = 40;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 30;
	#define RC_FILEVERSION 0,5,31,40
	#define RC_FILEVERSION_STRING "0, 5, 31, 40\0"
	static const char FULLVERSION_STRING[] = "0.5.31.40";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 31;
	

}
#endif //VERSION_H
