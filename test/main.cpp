/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <test-debug/debug.hpp>

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



#include <audio/river/river.hpp>
#include <audio/river/Manager.hpp>
#include <audio/river/Interface.hpp>
#include <gtest/gtest.h>
#include <etk/etk.hpp>
#include <etk/os/FSNode.hpp>
#include <cmath>
#include <sstream>

#include <thread>
#include "testAEC.hpp"
#include "testEchoDelay.hpp"
#include "testFormat.hpp"
#include "testMuxer.hpp"
#include "testPlaybackCallback.hpp"
#include "testPlaybackWrite.hpp"
#include "testRecordCallback.hpp"
#include "testRecordRead.hpp"
#include "testVolume.hpp"

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

