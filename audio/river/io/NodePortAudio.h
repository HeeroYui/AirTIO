/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __AUDIO_RIVER_IO_NODE_PORTAUDIO_H__
#define __AUDIO_RIVER_IO_NODE_PORTAUDIO_H__

#ifdef AUDIO_RIVER_BUILD_PORTAUDIO

#include <audio/river/Interface.h>
#include <audio/river/io/Node.h>
#include <portaudio/portaudio.h>

namespace audio {
	namespace river {
		namespace io {
			class Manager;
			//! @not-in-doc
			class NodePortAudio : public Node {
				protected:
					/**
					 * @brief Constructor
					 */
					NodePortAudio(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config);
				public:
					static std11::shared_ptr<NodePortAudio> create(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config);
					/**
					 * @brief Destructor
					 */
					virtual ~NodePortAudio();
					virtual bool isHarwareNode() {
						return true;
					};
				protected:
					PaStream* m_stream;
				public:
					int32_t duplexCallback(const void* _inputBuffer,
					                       const audio::Time& _timeInput,
					                       void* _outputBuffer,
					                       const audio::Time& _timeOutput,
					                       uint32_t _nbChunk,
					                       PaStreamCallbackFlags _status);
				protected:
					virtual void start();
					virtual void stop();
			};
		}
	}
}
#endif

#endif

