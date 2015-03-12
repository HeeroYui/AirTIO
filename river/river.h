/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_H__
#define __RIVER_H__

#include <etk/type.h>

namespace river {
	/**
	 * @brief Initialize the River Library
	 * @param[in] _filename Name of the configuration file (if "" ==> default config file)
	 */
	void init(const std::string& _filename);
	/**
	 * @brief Un-initialize the River Library
	 * @note this close all stream of all interfaces.
	 * @note really good for test.
	 */
	void unInit();
	/**
	 * @brief Get the status of initialisation
	 * @return true River is init
	 * @return false River is NOT init
	 */
	bool isInit();
}

#endif

