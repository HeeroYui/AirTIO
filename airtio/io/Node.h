/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __AIRTIO_IO_NODE_H__
#define __AIRTIO_IO_NODE_H__

#include <string>
#include <vector>
#include <list>
#include <stdint.h>
#include <chrono>
#include <functional>
#include <airtalgo/format.h>
#include <airtalgo/channel.h>
#include "Manager.h"
#include <memory>
#include <airtio/Interface.h>
#include <airtaudio/Interface.h>

namespace airtio {
	namespace io {
		class Manager;
		class Node {
			private:
				mutable std::mutex m_mutex;
			private:
				/**
				 * @brief Constructor
				 */
				Node(const std::string& _streamName, bool _isInput);
			public:
				static std::shared_ptr<Node> create(const std::string& _streamName, bool _isInput);
				/**
				 * @brief Destructor
				 */
				virtual ~Node();
			private:
				std::vector<std::shared_ptr<airtio::Interface> > m_list;
			public:
				void interfaceAdd(const std::shared_ptr<airtio::Interface>& _interface);
				void interfaceRemove(const std::shared_ptr<airtio::Interface>& _interface);
			private:
				airtaudio::Interface m_adac; //!< Real audio interface
				airtaudio::DeviceInfo m_info;
				unsigned int m_rtaudioFrameSize;
			public:
				int32_t rtAudioCallback(void* _outputBuffer,
				                        void * _inputBuffer,
				                        unsigned int _nBufferFrames,
				                        double _streamTime,
				                        airtaudio::streamStatus _status);
			private:
				std::string m_streamName;
			public:
				const std::string& getName() {
					return m_streamName;
				}
			private:
				std::vector<airtalgo::channel> m_map;
			public:
				const std::vector<airtalgo::channel>& getMap() {
					return m_map;
				}
			private:
				float m_frequency;
			public:
				float getFrequency() {
					return m_frequency;
				}
			private:
				airtalgo::format m_format;
			public:
				airtalgo::format getFormat() {
					return m_format;
				}
			private:
				bool m_isInput;
			public:
				bool isInput() {
					return m_isInput;
				}
				bool isOutput() {
					return !m_isInput;
				}
			private:
				void start();
				void stop();
		};
	}
}

#endif

