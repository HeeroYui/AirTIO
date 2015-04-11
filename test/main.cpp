/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "debug.h"
#include <audio/river/river.h>
#include <audio/river/Manager.h>
#include <audio/river/Interface.h>
#include <gtest/gtest.h>
#include <etk/etk.h>
#include <etk/os/FSNode.h>
#include <math.h>
#include <sstream>
#include <unistd.h>
#include <etk/thread.h>
#include "testAEC.h"
#include "testEchoDelay.h"
#include "testFormat.h"
#include "testMuxer.h"
#include "testPlaybackCallback.h"
#include "testPlaybackWrite.h"
#include "testRecordCallback.h"
#include "testRecordRead.h"
#include "testVolume.h"


#undef __class__
#define __class__ "test"

int main(int _argc, const char** _argv) {
	// init Google test :
	::testing::InitGoogleTest(&_argc, const_cast<char **>(_argv));
	// the only one init for etk:
	etk::init(_argc, _argv);
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (    data == "-h"
		     || data == "--help") {
			APPL_INFO("Help : ");
			APPL_INFO("    ./xxx ---");
			exit(0);
		}
	}
	return RUN_ALL_TESTS();
}

