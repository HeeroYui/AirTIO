/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <audio/river/debug.h>


int32_t audio::river::getLogId() {
	static int32_t g_val = etk::log::registerInstance("river");
	return g_val;
}
