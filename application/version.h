#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "06";
	static const char MONTH[] = "12";
	static const char YEAR[] = "2010";
	static const char UBUNTU_VERSION_STYLE[] = "10.12";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 3;
	static const long MINOR = 0;
	static const long BUILD = 5;
	static const long REVISION = 20;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 16;
	#define RC_FILEVERSION 3,0,5,20
	#define RC_FILEVERSION_STRING "3, 0, 5, 20\0"
	static const char FULLVERSION_STRING[] = "3.0.5.20";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 0;
	

}
#endif //VERSION_H
