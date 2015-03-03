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
		class NodeAirTAudio : public Node {
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
			protected:
				airtaudio::Interface m_adac; //!< Real audio interface
				airtaudio::DeviceInfo m_info;
				unsigned int m_rtaudioFrameSize;
			public:
				int32_t duplexCallback(const void* _inputBuffer,
				                       const std11::chrono::system_clock::time_point& _timeInput,
				                       void* _outputBuffer,
				                       const std11::chrono::system_clock::time_point& _timeOutput,
				                       uint32_t _nbChunk,
				                       const std::vector<airtaudio::status>& _status);
				int32_t recordCallback(const void* _inputBuffer,
				                       const std11::chrono::system_clock::time_point& _timeInput,
				                       uint32_t _nbChunk,
				                       const std::vector<airtaudio::status>& _status);
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

