/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */


#ifndef __AIRTIO_DEBUG_H__
#define __AIRTIO_DEBUG_H__

#include <etk/log.h>

namespace river {
	int32_t getLogId();
};
// TODO : Review this problem of multiple intanciation of "std::stringbuf sb"
#define AIRTIO_BASE(info,data) \
	do { \
		if (info <= etk::log::getLevel(river::getLogId())) { \
			std::stringbuf sb; \
			std::ostream tmpStream(&sb); \
			tmpStream << data; \
			etk::log::logStream(river::getLogId(), info, __LINE__, __class__, __func__, tmpStream); \
		} \
	} while(0)

#define AIRTIO_CRITICAL(data)      AIRTIO_BASE(1, data)
#define AIRTIO_ERROR(data)         AIRTIO_BASE(2, data)
#define AIRTIO_WARNING(data)       AIRTIO_BASE(3, data)
#ifdef DEBUG
	#define AIRTIO_INFO(data)          AIRTIO_BASE(4, data)
	#define AIRTIO_DEBUG(data)         AIRTIO_BASE(5, data)
	#define AIRTIO_VERBOSE(data)       AIRTIO_BASE(6, data)
	#define AIRTIO_TODO(data)          AIRTIO_BASE(4, "TODO : " << data)
#else
	#define AIRTIO_INFO(data)          do { } while(false)
	#define AIRTIO_DEBUG(data)         do { } while(false)
	#define AIRTIO_VERBOSE(data)       do { } while(false)
	#define AIRTIO_TODO(data)          do { } while(false)
#endif

#define AIRTIO_ASSERT(cond,data) \
	do { \
		if (!(cond)) { \
			AIRTIO_CRITICAL(data); \
			assert(!#cond); \
		} \
	} while (0)

#endif

