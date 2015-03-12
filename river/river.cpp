/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <river/river.h>
#include <river/debug.h>
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
	} else {
		RIVER_ERROR("River is already init not use : " << _filename);
	}
}

void river::initString(const std::string& _config) {
	if (river_isInit == false) {
		river_isInit = true;
		river_configFile = _config;
		RIVER_INFO("init RIVER with config ...");
		std11::shared_ptr<river::io::Manager> mng = river::io::Manager::getInstance();
		mng->initString(river_configFile);
	} else {
		RIVER_ERROR("River is already init not use Data ...");
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

