/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <etk/log.h>

namespace audio {
	namespace river {
		namespace widget {
			int32_t getLogId();
		}
	}
}

#define ARW_BASE(info,data) TK_LOG_BASE(audio::river::widget::getLogId(),info,data)

#define ARW_PRINT(data)         ARW_BASE(-1, data)
#define ARW_CRITICAL(data)      ARW_BASE(1, data)
#define ARW_ERROR(data)         ARW_BASE(2, data)
#define ARW_WARNING(data)       ARW_BASE(3, data)
#ifdef DEBUG
	#define ARW_INFO(data)          ARW_BASE(4, data)
	#define ARW_DEBUG(data)         ARW_BASE(5, data)
	#define ARW_VERBOSE(data)       ARW_BASE(6, data)
	#define ARW_TODO(data)          ARW_BASE(4, "TODO : " << data)
#else
	#define ARW_INFO(data)          do { } while(false)
	#define ARW_DEBUG(data)         do { } while(false)
	#define ARW_VERBOSE(data)       do { } while(false)
	#define ARW_TODO(data)          do { } while(false)
#endif

#define ARW_ASSERT(cond,data) \
	do { \
		if (!(cond)) { \
			ARW_CRITICAL(data); \
			assert(!#cond); \
		} \
	} while (0)

