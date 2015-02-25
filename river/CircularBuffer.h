/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */


#ifndef __RIVER_CIRCULAR_BUFFER_H__
#define __RIVER_CIRCULAR_BUFFER_H__

#include <etk/types.h>
#include <vector>
#include <etk/chrono.h>

namespace river {
	/**
	 * For these functions we have 4 solutions :
	 *  - Free Buffer
	 *             ----------------------------------------------------------
	 *      m_data |                            |                           |
	 *             ----------------------------------------------------------
	 *                                          m_write
	 *                                          m_read
	 *  - Full Buffer
	 *             ----------------------------------------------------------
	 *      m_data |****************************|***************************|
	 *             ----------------------------------------------------------
	 *                                          m_write
	 *                                          m_read
	 *  - Buffer in used
	 *             ----------------------------------------------------------
	 *      m_data |                |********************|                  |
	 *             ----------------------------------------------------------
	 *                              m_read                m_write
	 *  - Buffer out used
	 *             ----------------------------------------------------------
	 *      m_data |****************|                    |******************|
	 *             ----------------------------------------------------------
	 *                              m_read                m_write
	 */
	class CircularBuffer {
		private:
			std::vector<uint8_t> m_data; //!< data pointer
			void* m_write; //!< write pointer
			void* m_read; //!< read pointer
			std11::chrono::system_clock::time_point m_timeRead; //!< current read time
			uint32_t m_frequency;
			// TODO : Remove the m_size ==> this is a bad element to be mutex-less
			size_t m_size; //!< number of chunk availlable in this buffer
			size_t m_capacity; //!< number of chunk available in this Buffer
			size_t m_sizeChunk; //!< Size of one chunk (in byte)
		public:
			CircularBuffer();
			~CircularBuffer();
			/**
			 * @brief copy contructor.
			 * @param[in] _obj Circular buffer object
			 */
			CircularBuffer(const river::CircularBuffer& _obj);
			/**
			 * @brief copy operator.
			 * @param[in] _obj Circular buffer object
			 */
			CircularBuffer& operator=(const river::CircularBuffer& _obj);
			/**
			 * @brief set the capacity of the circular buffer.
			 * @param[in] _capacity Number of chunk in the buffer.
			 * @param[in] _chunkSize Size of one chunk.
			 * @param[in] _frequency Frequency of the buffer
			 */
			void setCapacity(size_t _capacity, size_t _chunkSize, uint32_t _frequency);
			/**
			 * @brief set the capacity of the circular buffer.
			 * @param[in] _capacity time in millisecond stored in the buffer.
			 * @param[in] _chunkSize Size of one chunk.
			 * @param[in] _frequency Frequency of the buffer
			 */
			void setCapacity(std11::chrono::milliseconds _capacity, size_t _chunkSize, uint32_t _frequency);
			/**
			 * @brief get free size of the buffer.
			 * @return Number of free chunk.
			 */
			size_t getFreeSize() const;
			/**
			 * @brief Get number of chunk in the buffer.
			 * @return number of chunk.
			 */
			size_t getSize() const {
				return m_size;
			}
			/**
			 * @brief Get number of chunk that can be set in the buffer.
			 * @return number of chunk.
			 */
			size_t getCapacity() const {
				return m_capacity;
			}
			/**
			 * @brief Write chunk in the buffer.
			 * @param[in] _data Pointer on the data.
			 * @param[in] _nbChunk number of chunk to copy.
			 * @param[in] _time Time to start write data (if before end ==> not replace data, write only if after end)
			 * @return Number of chunk copied.
			 */
			size_t write(const void* _data, size_t _nbChunk, const std11::chrono::system_clock::time_point& _time);
			/**
			 * @brief Read Chunk from the buffer to the pointer data.
			 * @param[out] _data Pointer on the data.
			 * @param[in] _nbChunk number of chunk to copy.
			 * @param[in] _time Time to start read data (if before start ==> add 0 at start, if after, remove unread data)
			 * @return Number of chunk copied.
			 */
			size_t read(void* _data, size_t _nbChunk, const std11::chrono::system_clock::time_point& _time);
			//! @previous
			size_t read(void* _data, size_t _nbChunk);
			void setReadPosition(const std11::chrono::system_clock::time_point& _time);
			
			std11::chrono::system_clock::time_point getReadTimeStamp() {
				return m_timeRead;
			}
			/**
			 * @brief Clear the buffer.
			 */
			void clear();
		private:
			/**
			 * @brief Get number of free chunks before end of buffer.
			 * @return Number of chunk.
			 */
			size_t getFreeSizeBeforEnd() const;
			/**
			 * @brief Get number of used chunks before end of buffer.
			 * @return Number of chunk.
			 */
			size_t getUsedSizeBeforEnd() const;
	};
}

#endif