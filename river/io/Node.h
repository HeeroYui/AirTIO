/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_IO_NODE_H__
#define __RIVER_IO_NODE_H__

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
#include <river/Interface.h>
#include <airtaudio/Interface.h>
#include <drain/IOFormatInterface.h>
#include <drain/Volume.h>
#include <etk/os/FSNode.h>

namespace river {
	namespace io {
		class Manager;
		class Node : public std::enable_shared_from_this<Node> {
			protected:
				/**
				 * @brief Constructor
				 */
				Node(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
			public:
				/**
				 * @brief Destructor
				 */
				virtual ~Node();
			
			protected:
				mutable std::mutex m_mutex;
				std::shared_ptr<const ejson::Object> m_config;
			protected:
				drain::Process m_process;
			public:
				const drain::IOFormatInterface& getInterfaceFormat() {
					if (m_isInput == true) {
						return m_process.getOutputConfig();
					} else {
						return m_process.getInputConfig();
					}
				}
				const drain::IOFormatInterface& getHarwareFormat() {
					if (m_isInput == true) {
						return m_process.getInputConfig();
					} else {
						return m_process.getOutputConfig();
					}
				}
			protected:
				
				std::shared_ptr<drain::VolumeElement> m_volume; //!< if a volume is set it is set here ...
			
			protected:
				std::vector<std::weak_ptr<river::Interface> > m_listAvaillable; //!< List of all interface that exist on this Node
				std::vector<std::shared_ptr<river::Interface> > m_list;
			public:
				void registerAsRemote(const std::shared_ptr<river::Interface>& _interface);
				void interfaceAdd(const std::shared_ptr<river::Interface>& _interface);
				void interfaceRemove(const std::shared_ptr<river::Interface>& _interface);
			protected:
				std::string m_name; //!< Harware.json configuration name
			public:
				const std::string& getName() {
					return m_name;
				}
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
				virtual void start() = 0;
				virtual void stop() = 0;
			public:
				const std::shared_ptr<drain::VolumeElement>& getVolume() {
					return m_volume;
				}
			public:
				void volumeChange();
			protected:
				int32_t newInput(const void* _inputBuffer,
				                 uint32_t _nbChunk,
				                 const std::chrono::system_clock::time_point& _time);
				int32_t newOutput(void* _outputBuffer,
				                  uint32_t _nbChunk,
				                  const std::chrono::system_clock::time_point& _time);
				
			public:
				virtual void generateDot(etk::FSNode& _node);
		};
	}
}

#endif

