/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <audio/river/river.hpp>
#include <audio/river/debug.hpp>
#include <audio/river/io/Manager.hpp>

static bool river_isInit = false;
static etk::String river_configFile = "";



void audio::river::init(const etk::String& _filename) {
	if (river_isInit == false) {
		river_isInit = true;
		river_configFile = _filename;
		RIVER_DEBUG("init RIVER :" << river_configFile);
		ememory::SharedPtr<audio::river::io::Manager> mng = audio::river::io::Manager::getInstance();
		if (mng != null) {
			mng->init(river_configFile);
		}
	} else {
		RIVER_ERROR("River is already init not use : " << _filename);
	}
}

void audio::river::initString(const etk::String& _config) {
	if (river_isInit == false) {
		river_isInit = true;
		river_configFile = _config;
		RIVER_DEBUG("init RIVER with config.");
		ememory::SharedPtr<audio::river::io::Manager> mng = audio::river::io::Manager::getInstance();
		if (mng != null) {
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
		if (mng != null) {
			RIVER_ERROR("Can not get on the RIVER hardware manager !!!");
			mng->unInit();
		}
	}
}

bool audio::river::isInit() {
	return river_isInit;
}

