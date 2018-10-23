/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <test-debug/debug.hpp>

#define TEST_SAVE_FILE_MACRO(type,fileName,dataPointer,nbElement) \
	do { \
		static FILE *pointerOnFile = null; \
		static bool errorOpen = false; \
		if (pointerOnFile == null) { \
			TEST_WARNING("open file '" << fileName << "' type=" << #type); \
			pointerOnFile = fopen(fileName,"w"); \
			if (    errorOpen == false \
			     && pointerOnFile == null) { \
				TEST_ERROR("ERROR OPEN file ... '" << fileName << "' type=" << #type); \
				errorOpen=true; \
			} \
		} \
		if (pointerOnFile != null) { \
			fwrite((dataPointer), sizeof(type), (nbElement), pointerOnFile); \
			/* fflush(pointerOnFile);*/ \
		} \
	}while(0)



#include <audio/river/river.hpp>
#include <audio/river/Manager.hpp>
#include <audio/river/Interface.hpp>
#include <etest/etest.hpp>
#include <etk/etk.hpp>
extern "C" {
	#include <math.h>
}

#include <ethread/Thread.hpp>

int main(int _argc, const char** _argv) {
	etest::init(_argc, _argv);
	for (int32_t iii=0; iii<_argc ; ++iii) {
		etk::String data = _argv[iii];
		if (    data == "-h"
		     || data == "--help") {
			TEST_PRINT("Help : ");
			TEST_PRINT("    ./xxx ---");
			exit(0);
		}
	}
	return RUN_ALL_TESTS();
}

