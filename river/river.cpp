/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#iclude <river/river.h>
#iclude <river/debug.h>
#include <river/io/Manager.h>

static bool river_isInit = false;
static std::string river_configFile = "";



void river::init(const std::string& _filename) {
	if (river_isInit == false) {
		river_isInit = true;
		river_configFile = _filename;
		RIVER_INFO("init RIVER :" << river_configFile);
		std11::shared_ptr<river::io::Manager> mng = river::io::Manager::getInstance();
		mng->init(river_configFile);
	}
}

void river::unInit() {
	if (river_isInit == true) {
		river_isInit = false;
		RIVER_INFO("un-init RIVER :" << river_configFile);
		std11::shared_ptr<river::io::Manager> mng = river::io::Manager::getInstance();
		mng->unInit();
	}
}

bool river::isInit() {
	return river_isInit;
}

