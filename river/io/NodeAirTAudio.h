/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_IO_NODE_AIRTAUDIO_H__
#define __RIVER_IO_NODE_AIRTAUDIO_H__

#ifdef __AIRTAUDIO_INFERFACE__

#include <river/io/Node.h>

namespace river {
	namespace io {
		class Manager;
		class Group;
		/**
		 * @brief Low level node that is manage on the interface with the extern lib airtaudio
		 */
		class NodeAirTAudio : public river::io::Node {
			friend class river::io::Group;
			protected:
				/**
				 * @brief Constructor
				 */
				NodeAirTAudio(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config);
			public:
				static std11::shared_ptr<NodeAirTAudio> create(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config);
				/**
				 * @brief Destructor
				 */
				virtual ~NodeAirTAudio();
				virtual bool isHarwareNode() {
					return true;
				};
			protected:
				airtaudio::Interface m_adac; //!< Real airtaudio interface
				airtaudio::DeviceInfo m_info; //!< information on the stream.
				unsigned int m_rtaudioFrameSize; // DEPRECATED soon...
			public:
				/**
				 * @brief Input Callback . Have recaive new data to process.
				 * @param[in] _inputBuffer Pointer on the data buffer.
				 * @param[in] _timeInput Time on the fist sample has been recorded.
				 * @param[in] _nbChunk Number of chunk in the buffer
				 * @param[in] _status DEPRECATED soon
				 * @return DEPRECATED soon
				 */
				int32_t recordCallback(const void* _inputBuffer,
				                       const std11::chrono::system_clock::time_point& _timeInput,
				                       uint32_t _nbChunk,
				                       const std::vector<airtaudio::status>& _status);
				/**
				 * @brief Playback callback. Request new data on output
				 * @param[in,out] _outputBuffer Pointer on the buffer to fill data.
				 * @param[in] _timeOutput Time on wich the data might be played.
				 * @param[in] _nbChunk Number of chunk in the buffer
				 * @param[in] _status DEPRECATED soon
				 * @return DEPRECATED soon
				 */
				int32_t playbackCallback(void* _outputBuffer,
				                         const std11::chrono::system_clock::time_point& _timeOutput,
				                         uint32_t _nbChunk,
				                         const std::vector<airtaudio::status>& _status);
			protected:
				virtual void start();
				virtual void stop();
		};
	}
}
#endif

#endif

