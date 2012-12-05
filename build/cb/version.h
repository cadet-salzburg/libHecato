#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "05";
	static const char MONTH[] = "12";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.12";
	
	//Software Status
	static const char STATUS[] = "Beta";
	static const char STATUS_SHORT[] = "b";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 0;
	static const long BUILD = 29;
	static const long REVISION = 28;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 29;
	#define RC_FILEVERSION 1,0,29,28
	#define RC_FILEVERSION_STRING "1, 0, 29, 28\0"
	static const char FULLVERSION_STRING[] = "1.0.29.28";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 29;
	

}
#endif //VERSION_H
