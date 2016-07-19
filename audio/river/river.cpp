/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <audio/river/river.h>
#include <audio/river/debug.h>
#include <audio/river/io/Manager.h>

static bool river_isInit = false;
static std::string river_configFile = "";



void audio::river::init(const std::string& _filename) {
	if (river_isInit == false) {
		river_isInit = true;
		river_configFile = _filename;
		RIVER_DEBUG("init RIVER :" << river_configFile);
		ememory::SharedPtr<audio::river::io::Manager> mng = audio::river::io::Manager::getInstance();
		if (mng != nullptr) {
			mng->init(river_configFile);
		}
	} else {
		RIVER_ERROR("River is already init not use : " << _filename);
	}
}

void audio::river::initString(const std::string& _config) {
	if (river_isInit == false) {
		river_isInit = true;
		river_configFile = _config;
		RIVER_DEBUG("init RIVER with config.");
		ememory::SharedPtr<audio::river::io::Manager> mng = audio::river::io::Manager::getInstance();
		if (mng != nullptr) {
			mng->initString(river_configFile);
		}
	} else {
		RIVER_ERROR("River is already init not use Data ...");
	}
}

void audio::river::unInit() {
	if (river_isInit == true) {
		river_isInit = false;
		RIVER_DEBUG("un-init RIVER.");
		ememory::SharedPtr<audio::river::io::Manager> mng = audio::river::io::Manager::getInstance();
		if (mng != nullptr) {
			RIVER_ERROR("Can not get on the RIVER hardware manager !!!");
			mng->unInit();
		}
	}
}

bool audio::river::isInit() {
	return river_isInit;
}

