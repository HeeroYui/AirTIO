/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_INTERFACE_H__
#define __RIVER_INTERFACE_H__

#include <string>
#include <vector>
#include <stdint.h>
#include <chrono>
#include <functional>
#include <mutex>
#include <audio/format.h>
#include <audio/channel.h>
#include <drain/Process.h>
#include <drain/EndPointCallback.h>
#include <drain/EndPointWrite.h>
#include <memory>

namespace river {
	namespace io {
		class Node;
	}
	class Interface : public std::enable_shared_from_this<Interface> {
		friend class io::Node;
		friend class Manager;
		protected:
			mutable std::recursive_mutex m_mutex;
		protected:
			std::shared_ptr<river::io::Node> m_node;
			float m_freq;
			std::vector<audio::channel> m_map;
			audio::format m_format;
			drain::Process m_process;
		protected:
			std::string m_name;
		public:
			virtual std::string getName() {
				return m_name;
			};
		protected:
			bool m_isInput;
		public:
			bool isInput() {
				return m_isInput;
			}
			bool isOutput() {
				return !m_isInput;
			}
		protected:
			/**
			 * @brief Constructor
			 */
			Interface();
			bool init(const std::string& _name,
			          float _freq,
			          const std::vector<audio::channel>& _map,
			          audio::format _format,
			          const std::shared_ptr<river::io::Node>& _node,
			          bool _isInput);
		public:
			/**
			 * @brief Destructor
			 */
			virtual ~Interface();
			static std::shared_ptr<Interface> create(const std::string& _name,
			                                         float _freq,
			                                         const std::vector<audio::channel>& _map,
			                                         audio::format _format,
			                                         const std::shared_ptr<river::io::Node>& _node,
			                                         bool _isInput);
			/**
			 * @brief set the read/write mode enable.
			 */
			virtual void setReadwrite();
			/**
			 * @brief When we want to implement a Callback Mode:
			 */
			virtual void setWriteCallback(drain::needDataFunctionWrite _function);
			virtual void setOutputCallback(size_t _chunkSize, drain::needDataFunction _function);
			virtual void setInputCallback(size_t _chunkSize, drain::haveNewDataFunction _function);
			/**
			 * @brief Add a volume group of the current channel.
			 * @note If you do not call this function with the group "FLOW" you chan not have a channel volume.
			 * @note the set volume stage can not be set after the start.
			 * @param[in] _name Name of the group classicle common group:
			 *                      - FLOW for channel volume.
			 *                      - MEDIA for multimedia volume control (audio player, player video, web streaming ...).
			 *                      - TTS for Test-to-speech volume control.
			 *                      - COMMUNICATION for user communication volume control.
			 *                      - NOTIFICATION for urgent notification volume control.
			 *                      - NOISE for small noise volume control.
			 */
			virtual void addVolumeGroup(const std::string& _name);
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
			 * @brief Set a parameter in the stream flow
			 * @param[in] _filter name of the filter (if you added some personels)
			 * @param[in] _parameter Parameter name.
			 * @param[in] _value Value to set.
			 * @return true set done
			 * @return false An error occured
			 * @example : setParameter("volume", "FLOW", "-3dB");
			 * @example : setParameter("LowPassFilter", "cutFrequency", "1000Hz");
			 */
			virtual bool setParameter(const std::string& _filter, const std::string& _parameter, const std::string& _value);
			/**
			 * @brief Get a parameter value
			 * @param[in] _filter name of the filter (if you added some personels)
			 * @param[in] _parameter Parameter name.
			 * @return The requested value.
			 * @example : getParameter("volume", "FLOW"); can return something like "-3dB"
			 * @example : getParameter("LowPassFilter", "cutFrequency"); can return something like "[-120..0]dB"
			 */
			virtual std::string getParameter(const std::string& _filter, const std::string& _parameter) const;
			/**
			 * @brief Get a parameter value
			 * @param[in] _filter name of the filter (if you added some personels)
			 * @param[in] _parameter Parameter name.
			 * @return The requested value.
			 * @example : getParameter("volume", "FLOW"); can return something like "[-120..0]dB"
			 * @example : getParameter("LowPassFilter", "cutFreqiency"); can return something like "]100..10000]Hz"
			 */
			virtual std::string getParameterProperty(const std::string& _filter, const std::string& _parameter) const;
			/**
			 * @brief write some audio sample in the speakers
			 * @param[in] _value Data To write on output
			 * @param[in] _nbChunk Number of audio chunk to write
			 */
			// TODO : TimeOut ???
			virtual void write(const void* _value, size_t _nbChunk);
			/**
			 * @brief read some audio sample from Microphone
			 * @param[in] _value Data To write on output
			 * @param[in] _nbChunk Number of audio chunk to write
			 */
			// TODO : TimeOut ???
			virtual void read(void* _value, size_t _nbChunk);
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
			virtual void systemNewInputData(std::chrono::system_clock::time_point _time, void* _data, size_t _nbChunk);
			virtual void systemNeedOutputData(std::chrono::system_clock::time_point _time, void* _data, size_t _nbChunk, size_t _chunkSize);
			virtual void systemVolumeChange();
			float m_volume; //!< Local channel Volume
	};
};


#endif

