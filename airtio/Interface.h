/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __AIRTIO_INTERFACE_H__
#define __AIRTIO_INTERFACE_H__

#include <string>
#include <vector>
#include <stdint.h>
#include <chrono>
#include <functional>
#include <mutex>
#include <airtalgo/format.h>
#include <airtalgo/channel.h>
#include <airtalgo/Process.h>
#include <airtalgo/EndPointCallback.h>
#include <memory>

namespace airtio {
	namespace io {
		class Node;
	}
	class Interface : public std::enable_shared_from_this<Interface> {
		friend class io::Node;
		friend class Manager;
		protected:
			mutable std::mutex m_mutex;
		protected:
			std::shared_ptr<airtio::io::Node> m_node;
			float m_freq;
			std::vector<airtalgo::channel> m_map;
			airtalgo::format m_format;
			std::shared_ptr<airtalgo::Process> m_process;
		protected:
			std::string m_name;
		public:
			virtual std::string getName() {
				return m_name;
			};
		protected:
			/**
			 * @brief Constructor
			 */
			Interface();
			bool init(const std::string& _name,
			          float _freq,
			          const std::vector<airtalgo::channel>& _map,
			          airtalgo::format _format,
			          const std::shared_ptr<airtio::io::Node>& _node);
		public:
			/**
			 * @brief Destructor
			 */
			virtual ~Interface();
			static std::shared_ptr<Interface> create(const std::string& _name,
			                                         float _freq,
			                                         const std::vector<airtalgo::channel>& _map,
			                                         airtalgo::format _format,
			                                         const std::shared_ptr<airtio::io::Node>& _node);
		public:
			/**
			 * @brief When we want to implement a Callback Mode :
			 */
			//virtual void setWriteCallback(size_t _chunkSize, writeNeedDataFunction_int16_t _function) {};
			virtual void setOutputCallback(size_t _chunkSize, airtalgo::needDataFunction _function, enum airtalgo::formatDataType _dataType);
			virtual void setInputCallback(size_t _chunkSize, airtalgo::haveNewDataFunction _function, enum airtalgo::formatDataType _dataType);
		public:
			/**
			 * @brief Start the Audio interface flow.
			 * @param[in] _time Time to start the flow (0) to start as fast as possible...
			 * @note _time to play buffer when output interface (if possible)
			 * @note _time to read buffer when inut interface (if possible)
			 */
			virtual void start(const std::chrono::system_clock::time_point& _time = std::chrono::system_clock::time_point());
			/**
			 * @brief Stop the current flow.
			 * @param[in] _fast The stream stop as fast as possible (not write all the buffer in speaker) but apply cross fade out.
			 * @param[in] _abort The stream stop whith no garenty of good audio stop.
			 */
			virtual void stop(bool _fast=false, bool _abort=false);
			/**
			 * @brief Abort flow (no audio garenty)
			 */
			virtual void abort();
			/**
			 * @brief Set the volume of this interface
			 * @param[in] _gainDB Gain in decibel to apply
			 */
			virtual void setVolume(float _gainDB);
			/**
			 * @brief Get the volume of this interface
			 * @return The gain in decibel applyied
			 */
			virtual float getVolume() const;
			/**
			 * @brief Get the volume range of this interface
			 * @return The gain in decibel range of this interface
			 */
			virtual std::pair<float,float> getVolumeRange() const;
			/**
			 * @brief write some audio sample in the speakers
			 * @param[in] _value Data To write on output
			 */
			// TODO : TimeOut ???
			virtual void write(const std::vector<int16_t>& _value);
			/**
			 * @brief write some audio sample in the speakers
			 * @param[in] _value Data To write on output
			 * @param[in] _nbChunk Number of audio chunk to write
			 */
			// TODO : TimeOut ???
			virtual void write(const int16_t* _value, size_t _nbChunk);
			/**
			 * @brief write some audio sample in the speakers
			 * @param[in] _value Data To write on output
			 */
			// TODO : TimeOut ???
			virtual std::vector<int16_t> read(size_t _nbChunk);
			/**
			 * @brief read some audio sample from Microphone
			 * @param[in] _value Data To write on output
			 * @param[in] _nbChunk Number of audio chunk to write
			 */
			// TODO : TimeOut ???
			virtual void read(const int16_t* _value, size_t _nbChunk);
			/**
			 * @brief Get number of chunk in the local buffer
			 * @return Number of chunk
			 */
			virtual size_t size() const;
			/**
			 * @brief Set buffer size in chunk number
			 * @param[in] _nbChunk Number of chunk in the buffer
			 */
			virtual void setBufferSize(size_t _nbChunk);
			/**
			 * @brief Set buffer size in chunk number
			 * @param[in] _nbChunk Number of chunk in the buffer
			 */
			virtual void setBufferSize(const std::chrono::duration<int64_t, std::micro>& _time);
			/**
			 * @brief Remove internal Buffer
			 */
			virtual void clearInternalBuffer();
			/**
			 * @brief Write : Get the time of the next sample time to write in the local buffer
			 * @brief Read : Get the time of the next sample time to read in the local buffer
			 */
			virtual std::chrono::system_clock::time_point getCurrentTime() const;
		private:
			virtual void systemNewInputData(std::chrono::system_clock::time_point _time, void* _data, int32_t _nbChunk);
			virtual void systemNeedOutputData(std::chrono::system_clock::time_point _time, void* _data, int32_t _nbChunk, size_t _chunkSize);
			
			std::vector<int8_t> m_data;
			float m_volume; //!< Local channel Volume
	};
};


#endif

