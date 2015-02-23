/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_IO_NODE_PORTAUDIO_H__
#define __RIVER_IO_NODE_PORTAUDIO_H__

#ifdef __PORTTAUDIO_INFERFACE__

#include <river/Interface.h>
#include <river/io/Node.h>
#include <portaudio.h>

namespace river {
	namespace io {
		class Manager;
		class NodePortAudio : public Node {
			protected:
				/**
				 * @brief Constructor
				 */
				NodePortAudio(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
			public:
				static std::shared_ptr<NodePortAudio> create(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
				/**
				 * @brief Destructor
				 */
				virtual ~NodePortAudio();
			protected:
				PaStream* m_stream;
			public:
				int32_t duplexCallback(const void* _inputBuffer,
				                       const std::chrono::system_clock::time_point& _timeInput,
				                       void* _outputBuffer,
				                       const std::chrono::system_clock::time_point& _timeOutput,
				                       uint32_t _nbChunk,
				                       PaStreamCallbackFlags _status);
			protected:
				virtual void start();
				virtual void stop();
		};
	}
}
#endif

#endif

