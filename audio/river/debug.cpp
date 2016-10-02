/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <audio/river/debug.hpp>


int32_t audio::river::getLogId() {
	static int32_t g_val = elog::registerInstance("river");
	return g_val;
}
