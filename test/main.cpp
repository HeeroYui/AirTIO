/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "debug.h"
#include <river/river.h>
#include <river/Manager.h>
#include <river/Interface.h>
#include <gtest/gtest.h>
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





/*
static void threadVolume() {
	std11::shared_ptr<river::Manager> manager;
	manager = river::Manager::create("testApplication");
	std11::shared_ptr<testCallbackVolume> process = std11::make_shared<testCallbackVolume>(manager);
	usleep(100000);
	process->run();
	process.reset();
	usleep(500000);
}

TEST(TestALL, testInputCallBackMicClean) {
	std11::shared_ptr<river::Manager> manager;
	manager = river::Manager::create("testApplication");
	
	std11::thread tmpThread(std11::bind(&threadVolume));
	usleep(30000);
	APPL_INFO("test input (callback mode)");
	std11::shared_ptr<testInCallback> process = std11::make_shared<testInCallback>(manager, "microphone-clean");
	process->run();
	
	usleep(100000);
	process.reset();
	usleep(500000);
	tmpThread.join();
}
*/


int main(int argc, char **argv) {
	// init Google test :
	::testing::InitGoogleTest(&argc, const_cast<char **>(argv));
	// the only one init for etk:
	etk::log::setLevel(etk::log::logLevelNone);
	for (int32_t iii=0; iii<argc ; ++iii) {
		std::string data = argv[iii];
		if (data == "-l0") {
			etk::log::setLevel(etk::log::logLevelNone);
		} else if (data == "-l1") {
			etk::log::setLevel(etk::log::logLevelCritical);
		} else if (data == "-l2") {
			etk::log::setLevel(etk::log::logLevelError);
		} else if (data == "-l3") {
			etk::log::setLevel(etk::log::logLevelWarning);
		} else if (data == "-l4") {
			etk::log::setLevel(etk::log::logLevelInfo);
		} else if (data == "-l5") {
			etk::log::setLevel(etk::log::logLevelDebug);
		} else if (data == "-l6") {
			etk::log::setLevel(etk::log::logLevelVerbose);
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_INFO("Help : ");
			APPL_INFO("    ./xxx [options]");
			APPL_INFO("        -l0: debug None");
			APPL_INFO("        -l1: debug Critical");
			APPL_INFO("        -l2: debug Error");
			APPL_INFO("        -l3: debug Warning");
			APPL_INFO("        -l4: debug Info");
			APPL_INFO("        -l5: debug Debug");
			APPL_INFO("        -l6: debug Verbose");
			APPL_INFO("        -h/--help: this help");
			exit(0);
		}
	}
	etk::setArgZero(argv[0]);
	etk::initDefaultFolder("exml_test");
	return RUN_ALL_TESTS();
}

