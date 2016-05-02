/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test-debug/debug.h>

#define TEST_SAVE_FILE_MACRO(type,fileName,dataPointer,nbElement) \
	do { \
		static FILE *pointerOnFile = nullptr; \
		static bool errorOpen = false; \
		if (pointerOnFile == nullptr) { \
			TEST_WARNING("open file '" << fileName << "' type=" << #type); \
			pointerOnFile = fopen(fileName,"w"); \
			if (    errorOpen == false \
			     && pointerOnFile == nullptr) { \
				TEST_ERROR("ERROR OPEN file ... '" << fileName << "' type=" << #type); \
				errorOpen=true; \
			} \
		} \
		if (pointerOnFile != nullptr) { \
			fwrite((dataPointer), sizeof(type), (nbElement), pointerOnFile); \
			/* fflush(pointerOnFile);*/ \
		} \
	}while(0)



#include <audio/river/river.h>
#include <audio/river/Manager.h>
#include <audio/river/Interface.h>
#include <gtest/gtest.h>
#include <etk/etk.h>
#include <etk/os/FSNode.h>
#include <math.h>
#include <sstream>
#include <unistd.h>
#include <thread>
#include "testAEC.h"
#include "testEchoDelay.h"
#include "testFormat.h"
#include "testMuxer.h"
#include "testPlaybackCallback.h"
#include "testPlaybackWrite.h"
#include "testRecordCallback.h"
#include "testRecordRead.h"
#include "testVolume.h"

int main(int _argc, const char** _argv) {
	// init Google test :
	::testing::InitGoogleTest(&_argc, const_cast<char **>(_argv));
	// the only one init for etk:
	etk::init(_argc, _argv);
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (    data == "-h"
		     || data == "--help") {
			TEST_PRINT("Help : ");
			TEST_PRINT("    ./xxx ---");
			exit(0);
		}
	}
	return RUN_ALL_TESTS();
}

