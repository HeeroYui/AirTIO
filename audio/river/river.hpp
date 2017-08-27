/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/types.hpp>
/**
 * @brief Audio library namespace
 */
namespace audio {
	/**
	 * @brief Audio-river library namespace
	 */
	namespace river {
		/**
		 * @brief Initialize the River Library
		 * @param[in] _filename Name of the configuration file (if "" ==> default config file)
		 */
		void init(const etk::String& _filename = "");
		/**
		 * @brief Initialize the River Library with a json data string
		 * @param[in] _config json sting data
		 */
		void initString(const etk::String& _config);
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
}

