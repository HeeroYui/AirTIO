/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <appl/debug.hpp>


int32_t appl::getLogId() {
	static int32_t g_val = elog::registerInstance("ioViewer");
	return g_val;
}
