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
#include <etk/chrono.h>
#include <etk/functional.h>
#include <etk/memory.h>
#include <audio/format.h>
#include <audio/channel.h>
#include "Manager.h"
#include <river/Interface.h>
#include <airtaudio/Interface.h>
#include <drain/IOFormatInterface.h>
#include <drain/Volume.h>
#include <etk/os/FSNode.h>

namespace river {
	namespace io {
		class Manager;
		class Node : public std11::enable_shared_from_this<Node> {
			protected:
				uint32_t m_uid; // uniqueNodeID
			protected:
				/**
				 * @brief Constructor
				 */
				Node(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config);
			public:
				/**
				 * @brief Destructor
				 */
				virtual ~Node();
			
			protected:
				mutable std11::mutex m_mutex;
				std11::shared_ptr<const ejson::Object> m_config;
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
				
				std11::shared_ptr<drain::VolumeElement> m_volume; //!< if a volume is set it is set here ...
			
			protected:
				std::vector<std11::weak_ptr<river::Interface> > m_listAvaillable; //!< List of all interface that exist on this Node
				std::vector<std11::shared_ptr<river::Interface> > m_list;
				size_t getNumberOfInterface(enum river::modeInterface _interfaceType);
			public:
				void registerAsRemote(const std11::shared_ptr<river::Interface>& _interface);
				void interfaceAdd(const std11::shared_ptr<river::Interface>& _interface);
				void interfaceRemove(const std11::shared_ptr<river::Interface>& _interface);
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
				const std11::shared_ptr<drain::VolumeElement>& getVolume() {
					return m_volume;
				}
			public:
				void volumeChange();
			protected:
				int32_t newInput(const void* _inputBuffer,
				                 uint32_t _nbChunk,
				                 const std11::chrono::system_clock::time_point& _time);
				int32_t newOutput(void* _outputBuffer,
				                  uint32_t _nbChunk,
				                  const std11::chrono::system_clock::time_point& _time);
				
			public:
				virtual void generateDot(etk::FSNode& _node);
		};
	}
}

#endif

