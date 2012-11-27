#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "04";
	static const char MONTH[] = "09";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.09";
	
	//Software Status
	static const char STATUS[] = "Beta";
	static const char STATUS_SHORT[] = "b";
	
	//Standard Version Type
	static const long MAJOR = 0;
	static const long MINOR = 1;
	static const long BUILD = 54;
	static const long REVISION = 1;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 73;
	#define RC_FILEVERSION 0,1,54,1
	#define RC_FILEVERSION_STRING "0, 1, 54, 1\0"
	static const char FULLVERSION_STRING[] = "0.1.54.1";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 54;
	

}
#endif //VERSION_H
