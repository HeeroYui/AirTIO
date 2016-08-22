/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <elog/log.h>

namespace audio {
	namespace river {
		int32_t getLogId();
	}
}
#define RIVER_BASE(info,data)  ELOG_BASE(audio::river::getLogId(),info,data)

#define RIVER_PRINT(data)         RIVER_BASE(-1, data)
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


#define RIVER_SAVE_FILE_MACRO(type,fileName,dataPointer,nbElement) \
	do { \
		static FILE *pointerOnFile = nullptr; \
		static bool errorOpen = false; \
		if (pointerOnFile == nullptr) { \
			RIVER_WARNING("open file '" << fileName << "' type=" << #type); \
			pointerOnFile = fopen(fileName,"w"); \
			if (    errorOpen == false \
			     && pointerOnFile == nullptr) { \
				RIVER_ERROR("ERROR OPEN file ... '" << fileName << "' type=" << #type); \
				errorOpen=true; \
			} \
		} \
		if (pointerOnFile != nullptr) { \
			fwrite((dataPointer), sizeof(type), (nbElement), pointerOnFile); \
			/* fflush(pointerOnFile);*/ \
		} \
	}while(0)

