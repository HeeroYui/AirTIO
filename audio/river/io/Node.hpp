/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/String.hpp>
#include <etk/Vector.hpp>
#include <etk/Function.hpp>
#include <ememory/memory.hpp>
#include <audio/format.hpp>
#include <audio/channel.hpp>
#include "Manager.hpp"
#include <audio/river/Interface.hpp>
#include <audio/drain/IOFormatInterface.hpp>
#include <audio/drain/Volume.hpp>
#include <etk/io/Interface.hpp>

namespace audio {
	namespace river {
		namespace io {
			class Manager;
			class Group;
			/**
			 * @brief A node is the base for input/output interface. When a output id declared, we automaticly have a feedback associated.
			 * this manage the muxing of data for output an the demuxing for input.
			 */
			class Node : public ememory::EnableSharedFromThis<Node> {
				friend class audio::river::io::Group;
				protected:
					uint32_t m_uid; //!< uniqueNodeID use for debug an dot generation.
				protected:
					/**
					 * @brief Constructor
					 * @param[in] _name Name of the node.
					 * @param[in] _config Configuration of the node.
					 */
					Node(const etk::String& _name, const ejson::Object& _config);
				public:
					/**
					 * @brief Destructor
					 */
					virtual ~Node();
					/**
					 * @brief Get the status of this node acces on harware or acces on other node (virtual).
					 * @return true This is an harware interface.
					 * @return false this is a virtual interface.
					 */
					virtual bool isHarwareNode() {
						return false;
					};
				protected:
					mutable ethread::Mutex m_mutex; //!< prevent open/close/write/read access that is multi-threaded.
					const ejson::Object m_config; //!< configuration description.
				protected:
					audio::drain::Process m_process; //!< Low level algorithms
				public:
					/**
					 * @brief Get the uper client interface configuration.
					 * @return process configuration.
					 */
					const audio::drain::IOFormatInterface& getInterfaceFormat() {
						if (m_isInput == true) {
							return m_process.getOutputConfig();
						} else {
							return m_process.getInputConfig();
						}
					}
					/**
					 * @brief Get the harware client interface configuration.
					 * @return process configuration.
					 */
					const audio::drain::IOFormatInterface& getHarwareFormat() {
						if (m_isInput == true) {
							return m_process.getInputConfig();
						} else {
							return m_process.getOutputConfig();
						}
					}
				protected:
					ememory::SharedPtr<audio::drain::VolumeElement> m_volume; //!< if a volume is set it is set here ... for hardware interface only.
				protected:
					etk::Vector<ememory::WeakPtr<audio::river::Interface> > m_listAvaillable; //!< List of all interface that exist on this Node
					etk::Vector<ememory::SharedPtr<audio::river::Interface> > m_list; //!< List of all connected interface at this node.
					/**
					 * @brief Get the number of interface with a specific type.
					 * @param[in] _interfaceType Type of the interface.
					 * @return Number of interface connected.
					 */
					size_t getNumberOfInterface(enum audio::river::modeInterface _interfaceType);
					/**
					 * @brief Get the number of interface with a specific type that can connect on the Node.
					 * @param[in] _interfaceType Type of the interface.
					 * @return Number of interface that can connect.
					 */
					size_t getNumberOfInterfaceAvaillable(enum audio::river::modeInterface _interfaceType);
				public:
					/**
					 * @brief Get the number of interface connected
					 * @return Number of interfaces.
					 */
					size_t getNumberOfInterface() {
						return m_list.size();
					}
				public:
					/**
					 * @brief Register an interface that can connect on it. (might be done in the Interface Init)
					 * @note We keep a ememory::WeakPtr. this is the reason why we do not have a remove.
					 * @param[in] _interface Pointer on the interface to register.
					 */
					void registerAsRemote(const ememory::SharedPtr<audio::river::Interface>& _interface);
					/**
					 * @brief Request this interface might receve/send dat on the flow. (start/resume)
					 * @param[in] _interface Pointer on the interface to register.
					 */
					void interfaceAdd(const ememory::SharedPtr<audio::river::Interface>& _interface);
					/**
					 * @brief Un-register the interface as an availlable read/write interface. (suspend/stop)
					 * @param[in] _interface Pointer on the interface to register.
					 */
					void interfaceRemove(const ememory::SharedPtr<audio::river::Interface>& _interface);
				protected:
					etk::String m_name; //!< Name of the interface
				public:
					/**
					 * @brief Get the interface name.
					 * @return Current name.
					 */
					const etk::String& getName() {
						return m_name;
					}
				protected:
					bool m_isInput; //!< sense of the stream
				public:
					/**
					 * @brief Check if it is an input stream
					 * @return true if it is an input/ false otherwise
					 */
					bool isInput() {
						return m_isInput;
					}
					/**
					 * @brief Check if it is an output stream
					 * @return true if it is an output/ false otherwise
					 */
					bool isOutput() {
						return !m_isInput;
					}
				protected:
					ememory::WeakPtr<audio::river::io::Group> m_group; //!< reference on the group. If available.
				public:
					/**
					 * @brief Set this node in a low level group.
					 * @param[in] _group Group reference.
					 */
					void setGroup(ememory::SharedPtr<audio::river::io::Group> _group) {
						m_group = _group;
					}
				protected:
					/**
					 * @brief Start the flow in the group (start if no group)
					 */
					void startInGroup();
					/**
					 * @brief Stop the flow in the group (stop if no group)
					 */
					void stopInGroup();
					/**
					 * @brief Real start of the stream
					 */
					virtual void start() = 0;
					/**
					 * @brief Real stop of the stream
					 */
					virtual void stop() = 0;
				public:
					/**
					 * @brief If this iss an hardware interface we can have a resuest of the volume stage:
					 * @return pointer on the requested volume.
					 */
					const ememory::SharedPtr<audio::drain::VolumeElement>& getVolume() {
						return m_volume;
					}
				public:
					/**
					 * @brief Called when a group wolume has been change to update all volume stage.
					 */
					void volumeChange();
				protected:
					/**
					 * @brief Call by child classes to process data in all interface linked on the current Node. Have new input to process.
					 * @param[in] _inputBuffer Pointer on the data.
					 * @param[in] _nbChunk Number of chunk in the buffer.
					 * @param[in] _time Time where the first sample has been capture.
					 */
					void newInput(const void* _inputBuffer,
					              uint32_t _nbChunk,
					              const audio::Time& _time);
					/**
					 * @brief Call by child classes to process data in all interface linked on the current Node. Have new output to get. this call the feedback too.
					 * @param[in,out] _outputBuffer Pointer on the buffer to write the data.
					 * @param[in] _nbChunk Number of chunk to write in the buffer.
					 * @param[in] _time Time where the data might be played.
					 */
					void newOutput(void* _outputBuffer,
					               uint32_t _nbChunk,
					               const audio::Time& _time);
				public:
					/**
					 * @brief Generate the node dot file section
					 * @param[in] _io File interface to dump data.
					 */
					virtual void generateDot(ememory::SharedPtr<etk::io::Interface>& _io);
			};
		}
	}
}

