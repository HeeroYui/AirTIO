/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <airtio/debug.h>


int32_t airtio::getLogId() {
	static int32_t g_val = etk::log::registerInstance("airtio");
	return g_val;
}
