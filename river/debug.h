/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */


#ifndef __RIVER_DEBUG_H__
#define __RIVER_DEBUG_H__

#include <etk/log.h>

namespace river {
	int32_t getLogId();
};
// TODO : Review this problem of multiple intanciation of "std::stringbuf sb"
#define RIVER_BASE(info,data) \
	do { \
		if (info <= etk::log::getLevel(river::getLogId())) { \
			std::stringbuf sb; \
			std::ostream tmpStream(&sb); \
			tmpStream << data; \
			etk::log::logStream(river::getLogId(), info, __LINE__, __class__, __func__, tmpStream); \
		} \
	} while(0)

#define RIVER_CRITICAL(data)      RIVER_BASE(1, data)
#define RIVER_ERROR(data)         RIVER_BASE(2, data)
#define RIVER_WARNING(data)       RIVER_BASE(3, data)
#ifdef DEBUG
	#define RIVER_INFO(data)          RIVER_BASE(4, data)
	#define RIVER_DEBUG(data)         RIVER_BASE(5, data)
	#define RIVER_VERBOSE(data)       RIVER_BASE(6, data)
	#define RIVER_TODO(data)          RIVER_BASE(4, "TODO : " << data)
#else
	#define RIVER_INFO(data)          do { } while(false)
	#define RIVER_DEBUG(data)         do { } while(false)
	#define RIVER_VERBOSE(data)       do { } while(false)
	#define RIVER_TODO(data)          do { } while(false)
#endif

#define RIVER_ASSERT(cond,data) \
	do { \
		if (!(cond)) { \
			RIVER_CRITICAL(data); \
			assert(!#cond); \
		} \
	} while (0)

#endif

