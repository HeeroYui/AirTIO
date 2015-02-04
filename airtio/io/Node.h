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
#include <audio/format.h>
#include <audio/channel.h>
#include "Manager.h"
#include <memory>
#include <airtio/Interface.h>
#include <airtaudio/Interface.h>
#include <airtalgo/IOFormatInterface.h>
#include <airtalgo/Volume.h>

namespace airtio {
	namespace io {
		class Manager;
		class Node {
			private:
				mutable std::mutex m_mutex;
				std::shared_ptr<const ejson::Object> m_config;
				std::shared_ptr<airtalgo::VolumeElement> m_volume; //!< if a volume is set it is set here ...
			private:
				/**
				 * @brief Constructor
				 */
				Node(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
			public:
				static std::shared_ptr<Node> create(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
				/**
				 * @brief Destructor
				 */
				virtual ~Node();
			private:
				std::vector<std::weak_ptr<airtio::Interface> > m_listAvaillable; //!< List of all interface that exist on this Node
				std::vector<std::shared_ptr<airtio::Interface> > m_list;
			public:
				void registerAsRemote(const std::shared_ptr<airtio::Interface>& _interface);
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
				std::string m_name; //!< Harware.json configuration name
			public:
				const std::string& getName() {
					return m_name;
				}
			private:
				airtalgo::IOFormatInterface m_interfaceFormat;
			public:
				const airtalgo::IOFormatInterface& getInterfaceFormat() {
					return m_interfaceFormat;
				}
			private:
				airtalgo::IOFormatInterface m_hardwareFormat;
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
			public:
				const std::shared_ptr<airtalgo::VolumeElement>& getVolume() {
					return m_volume;
				}
			public:
				void volumeChange();
		};
	}
}

#endif

