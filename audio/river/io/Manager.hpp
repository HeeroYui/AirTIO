/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/String.hpp>
#include <etk/Vector.hpp>
#include <etk/Map.hpp>
#include <list>
#include <cstdint>
#include <mutex>
#include <chrono>
#include <functional>
#include <ememory/memory.hpp>
#include <audio/format.hpp>
#include <audio/channel.hpp>
#include <ejson/ejson.hpp>
#include <audio/drain/Volume.hpp>
#include <audio/river/io/Group.hpp>

namespace audio {
	namespace river {
		namespace io {
			class Node;
			/**
			 * @brief Internal sigleton of all Flow hadware and virtuals.
			 * @note this class will be initialize by the audio::river::init() function at the start of the application.
			 */
			class Manager : public ememory::EnableSharedFromThis<Manager> {
				private:
					mutable std::recursive_mutex m_mutex; //!< prevent multiple access
				private:
					/**
					 * @brief Constructor
					 */
					Manager();
				public:
					static ememory::SharedPtr<Manager> getInstance();
					/**
					 * @brief Destructor
					 */
					~Manager();
					/**
					 * @brief Called by audio::river::init() to set the hardware configuration file.
					 * @param[in] _filename Name of the file to initialize.
					 */
					void init(const etk::String& _filename);
					/**
					 * @brief Called by audio::river::initString() to set the hardware configuration string.
					 * @param[in] _data json configuration string.
					 */
					void initString(const etk::String& _data);
					/**
					 * @brief Called by audio::river::inInit() to uninitialize all the low level interface.
					 */
					void unInit();
				private:
					ejson::Document m_config; //!< harware configuration
					etk::Vector<ememory::SharedPtr<audio::river::io::Node> > m_listKeepAlive; //!< list of all Node that might be keep alive sone/all time
					etk::Vector<ememory::WeakPtr<audio::river::io::Node> > m_list; //!< List of all IO node
				public:
					/**
					 * @brief Get a node with his name (the name is set in the description file.
					 * @param[in] _name Name of the node
					 * @return Pointer on the noe or a nullptr if the node does not exist in the file or an error occured.
					 */
					ememory::SharedPtr<audio::river::io::Node> getNode(const etk::String& _name);
				private:
					etk::Vector<ememory::SharedPtr<audio::drain::VolumeElement> > m_volumeGroup; //!< List of All global volume in the Low level interface.
				public:
					/**
					 * @brief Get a volume in the global list of vilume
					 * @param[in] _name Name of the volume.
					 * @return pointer on the requested volume (create it if does not exist). nullptr if the name is empty.
					 */
					ememory::SharedPtr<audio::drain::VolumeElement> getVolumeGroup(const etk::String& _name);
					/**
					 * @brief Get all input audio stream.
					 * @return a list of all availlables input stream name
					 */
					etk::Vector<etk::String> getListStreamInput();
					/**
					 * @brief Get all output audio stream.
					 * @return a list of all availlables output stream name
					 */
					etk::Vector<etk::String> getListStreamOutput();
					/**
					 * @brief Get all audio virtual stream.
					 * @return a list of all availlables virtual stream name
					 */
					etk::Vector<etk::String> getListStreamVirtual();
					/**
					 * @brief Get all audio stream.
					 * @return a list of all availlables stream name
					 */
					etk::Vector<etk::String> getListStream();
					
					/**
					 * @brief Set a volume for a specific group
					 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
					 * @param[in] _value Volume in dB to set.
					 * @return true set done
					 * @return false An error occured
					 * @example : setVolume("MASTER", -3.0f);
					 */
					bool setVolume(const etk::String& _volumeName, float _valuedB);
					/**
					 * @brief Get a volume value
					 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
					 * @return The Volume value in dB.
					 * @example ret = getVolume("MASTER"); can return something like ret = -3.0f
					 */
					float getVolume(const etk::String& _volumeName);
					/**
					 * @brief Get a parameter value
					 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
					 * @return The requested value Range.
					 * @example ret = getVolumeRange("MASTER"); can return something like ret=(-120.0f,0.0f)
					 */
					etk::Pair<float,float> getVolumeRange(const etk::String& _volumeName) const;
					/**
					 * @brief Set a Mute for a specific volume group
					 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
					 * @param[in] _mute Mute enable or disable.
					 */
					void setMute(const etk::String& _volumeName, bool _mute);
					/**
					 * @brief Get a volume value
					 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
					 * @return The Mute of the volume volume.
					 */
					bool getMute(const etk::String& _volumeName);
					/**
					 * @brief Generate the dot file corresponding at the actif nodes.
					 * @param[in] _filename Name of the file to write data.
					 */
					void generateDot(const etk::String& _filename);
				private:
					etk::Map<etk::String, ememory::SharedPtr<audio::river::io::Group> > m_listGroup; //!< List of all groups
					/**
					 * @brief get a low level interface group.
					 * @param[in] _name Name of the group.
					 * @return Pointer on the requested group or nullptr if the group does not existed.
					 */
					ememory::SharedPtr<audio::river::io::Group> getGroup(const etk::String& _name);
					
			};
		}
	}
}

