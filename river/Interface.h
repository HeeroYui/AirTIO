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
#include <etk/mutex.h>
#include <etk/chrono.h>
#include <etk/functional.h>
#include <etk/memory.h>
#include <audio/format.h>
#include <audio/channel.h>
#include <drain/Process.h>
#include <drain/EndPointCallback.h>
#include <drain/EndPointWrite.h>
#include <ejson/ejson.h>
#include <etk/os/FSNode.h>

namespace river {
	namespace io {
		class Node;
		class NodeAirTAudio;
		class NodeAEC;
		class NodeMuxer;
	}
	enum modeInterface {
		modeInterface_unknow,
		modeInterface_input,
		modeInterface_output,
		modeInterface_feedback,
	};
	class Interface : public std11::enable_shared_from_this<Interface> {
		friend class io::Node;
		friend class io::NodeAirTAudio;
		friend class io::NodeAEC;
		friend class io::NodeMuxer;
		friend class Manager;
		protected:
			uint32_t m_uid; //!< unique ID for interface
		protected:
			/**
			 * @brief Constructor
			 */
			Interface();
			bool init(float _freq,
			          const std::vector<audio::channel>& _map,
			          audio::format _format,
			          const std11::shared_ptr<river::io::Node>& _node,
			          const std11::shared_ptr<const ejson::Object>& _config);
			static std11::shared_ptr<Interface> create(float _freq,
			                                           const std::vector<audio::channel>& _map,
			                                           audio::format _format,
			                                           const std11::shared_ptr<river::io::Node>& _node,
			                                           const std11::shared_ptr<const ejson::Object>& _config);
		public:
			/**
			 * @brief Destructor
			 */
			virtual ~Interface();
		protected:
			mutable std11::recursive_mutex m_mutex;
			std11::shared_ptr<const ejson::Object> m_config;
		protected:
			enum modeInterface m_mode;
		public:
			enum modeInterface getMode() {
				return m_mode;
			}
			drain::Process m_process;
		public:
			const drain::IOFormatInterface& getInterfaceFormat() {
				if (    m_mode == modeInterface_input
				     || m_mode == modeInterface_feedback) {
					return m_process.getOutputConfig();
				} else {
					return m_process.getInputConfig();
				}
			}
		
		protected:
			std11::shared_ptr<river::io::Node> m_node;
		protected:
			std::string m_name;
		public:
			virtual std::string getName() {
				return m_name;
			};
			virtual void setName(const std::string& _name) {
				m_name = _name;
			};
			/**
			 * @brief set the read/write mode enable.
			 */
			virtual void setReadwrite();
			/**
			 * @brief When we want to implement a Callback Mode:
			 */
			virtual void setWriteCallback(drain::playbackFunctionWrite _function);
			virtual void setOutputCallback(drain::playbackFunction _function);
			virtual void setInputCallback(drain::recordFunction _function);
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
			virtual void start(const std11::chrono::system_clock::time_point& _time = std11::chrono::system_clock::time_point());
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
			virtual void setBufferSize(const std11::chrono::microseconds& _time);
			/**
			 * @brief Remove internal Buffer
			 */
			virtual void clearInternalBuffer();
			/**
			 * @brief Write : Get the time of the next sample time to write in the local buffer
			 * @brief Read : Get the time of the next sample time to read in the local buffer
			 */
			virtual std11::chrono::system_clock::time_point getCurrentTime() const;
		private:
			virtual void systemNewInputData(std11::chrono::system_clock::time_point _time, const void* _data, size_t _nbChunk);
			virtual void systemNeedOutputData(std11::chrono::system_clock::time_point _time, void* _data, size_t _nbChunk, size_t _chunkSize);
			virtual void systemVolumeChange();
		public:
			virtual void generateDot(etk::FSNode& _node, const std::string& _nameIO, bool _isLink=true);
			virtual std::string getDotNodeName() const;
	};
};


#endif

