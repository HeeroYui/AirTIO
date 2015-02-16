/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_IO_NODE_AIRTAUDIO_H__
#define __RIVER_IO_NODE_AIRTAUDIO_H__

#include <river/io/Node.h>

namespace river {
	namespace io {
		class Manager;
		class NodeAirTAudio : public Node {
			protected:
				/**
				 * @brief Constructor
				 */
				NodeAirTAudio(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
			public:
				static std::shared_ptr<NodeAirTAudio> create(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
				/**
				 * @brief Destructor
				 */
				virtual ~NodeAirTAudio();
			protected:
				airtaudio::Interface m_adac; //!< Real audio interface
				airtaudio::DeviceInfo m_info;
				unsigned int m_rtaudioFrameSize;
			public:
				int32_t airtAudioCallback(void* _outputBuffer,
				                          void * _inputBuffer,
				                          uint32_t _nbChunk,
				                          const std::chrono::system_clock::time_point& _time,
				                          airtaudio::status _status);
			protected:
				virtual void start();
				virtual void stop();
		};
	}
}

#endif

