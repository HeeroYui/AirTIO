/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#ifdef AUDIO_RIVER_BUILD_ORCHESTRA

#include <audio/river/io/Node.h>
#include <audio/orchestra/Interface.h>

namespace audio {
	namespace river {
		namespace io {
			class Manager;
			class Group;
			/**
			 * @brief Low level node that is manage on the interface with the extern lib airtaudio
			 */
			class NodeOrchestra : public audio::river::io::Node {
				friend class audio::river::io::Group;
				protected:
					/**
					 * @brief Constructor
					 */
					NodeOrchestra(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
				public:
					static std::shared_ptr<NodeOrchestra> create(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
					/**
					 * @brief Destructor
					 */
					virtual ~NodeOrchestra();
					virtual bool isHarwareNode() {
						return true;
					};
				protected:
					audio::orchestra::Interface m_interface; //!< Real airtaudio interface
					audio::orchestra::DeviceInfo m_info; //!< information on the stream.
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
					                       const audio::Time& _timeInput,
					                       uint32_t _nbChunk,
					                       const std::vector<audio::orchestra::status>& _status);
					/**
					 * @brief Playback callback. Request new data on output
					 * @param[in,out] _outputBuffer Pointer on the buffer to fill data.
					 * @param[in] _timeOutput Time on wich the data might be played.
					 * @param[in] _nbChunk Number of chunk in the buffer
					 * @param[in] _status DEPRECATED soon
					 * @return DEPRECATED soon
					 */
					int32_t playbackCallback(void* _outputBuffer,
					                         const audio::Time& _timeOutput,
					                         uint32_t _nbChunk,
					                         const std::vector<audio::orchestra::status>& _status);
				protected:
					virtual void start();
					virtual void stop();
			};
		}
	}
}
#endif
