/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __AUDIO_RIVER_IO_MANAGER_H__
#define __AUDIO_RIVER_IO_MANAGER_H__

#include <string>
#include <vector>
#include <map>
#include <list>
#include <stdint.h>
#include <mutex>
#include <chrono>
#include <functional>
#include <memory>
#include <audio/format.h>
#include <audio/channel.h>
#include <ejson/ejson.h>
#include <audio/drain/Volume.h>
#include <audio/river/io/Group.h>

namespace audio {
	namespace river {
		namespace io {
			class Node;
			/**
			 * @brief Internal sigleton of all Flow hadware and virtuals.
			 * @note this class will be initialize by the audio::river::init() function at the start of the application.
			 */
			class Manager : public std11::enable_shared_from_this<Manager> {
				private:
					mutable std11::recursive_mutex m_mutex; //!< prevent multiple access
				private:
					/**
					 * @brief Constructor
					 */
					Manager();
				public:
					static std11::shared_ptr<Manager> getInstance();
					/**
					 * @brief Destructor
					 */
					~Manager();
					/**
					 * @brief Called by audio::river::init() to set the hardware configuration file.
					 * @param[in] _filename Name of the file to initialize.
					 */
					void init(const std::string& _filename);
					/**
					 * @brief Called by audio::river::initString() to set the hardware configuration string.
					 * @param[in] _data json configuration string.
					 */
					void initString(const std::string& _data);
					/**
					 * @brief Called by audio::river::inInit() to uninitialize all the low level interface.
					 */
					void unInit();
				private:
					ejson::Document m_config; //!< harware configuration
					std::vector<std11::shared_ptr<audio::river::io::Node> > m_listKeepAlive; //!< list of all Node that might be keep alive sone/all time
					std::vector<std11::weak_ptr<audio::river::io::Node> > m_list; //!< List of all IO node
				public:
					/**
					 * @brief Get a node with his name (the name is set in the description file.
					 * @param[in] _name Name of the node
					 * @return Pointer on the noe or a nullptr if the node does not exist in the file or an error occured.
					 */
					std11::shared_ptr<audio::river::io::Node> getNode(const std::string& _name);
				private:
					std::vector<std11::shared_ptr<audio::drain::VolumeElement> > m_volumeGroup; //!< List of All global volume in the Low level interface.
				public:
					/**
					 * @brief Get a volume in the global list of vilume
					 * @param[in] _name Name of the volume.
					 * @return pointer on the requested volume (create it if does not exist). nullptr if the name is empty.
					 */
					std11::shared_ptr<audio::drain::VolumeElement> getVolumeGroup(const std::string& _name);
					/**
					 * @brief Get all input audio stream.
					 * @return a list of all availlables input stream name
					 */
					std::vector<std::string> getListStreamInput();
					/**
					 * @brief Get all output audio stream.
					 * @return a list of all availlables output stream name
					 */
					std::vector<std::string> getListStreamOutput();
					/**
					 * @brief Get all audio virtual stream.
					 * @return a list of all availlables virtual stream name
					 */
					std::vector<std::string> getListStreamVirtual();
					/**
					 * @brief Get all audio stream.
					 * @return a list of all availlables stream name
					 */
					std::vector<std::string> getListStream();
					
					/**
					 * @brief Set a volume for a specific group
					 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
					 * @param[in] _value Volume in dB to set.
					 * @return true set done
					 * @return false An error occured
					 * @example : setVolume("MASTER", -3.0f);
					 */
					bool setVolume(const std::string& _volumeName, float _valuedB);
					/**
					 * @brief Get a volume value
					 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
					 * @return The Volume value in dB.
					 * @example ret = getVolume("MASTER"); can return something like ret = -3.0f
					 */
					float getVolume(const std::string& _volumeName);
					/**
					 * @brief Get a parameter value
					 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
					 * @return The requested value Range.
					 * @example ret = getVolumeRange("MASTER"); can return something like ret=(-120.0f,0.0f)
					 */
					std::pair<float,float> getVolumeRange(const std::string& _volumeName) const;
					/**
					 * @brief Set a Mute for a specific volume group
					 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
					 * @param[in] _mute Mute enable or disable.
					 */
					void setMute(const std::string& _volumeName, bool _mute);
					/**
					 * @brief Get a volume value
					 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
					 * @return The Mute of the volume volume.
					 */
					bool getMute(const std::string& _volumeName);
					/**
					 * @brief Generate the dot file corresponding at the actif nodes.
					 * @param[in] _filename Name of the file to write data.
					 */
					void generateDot(const std::string& _filename);
				private:
					std::map<std::string, std11::shared_ptr<audio::river::io::Group> > m_listGroup; //!< List of all groups
					/**
					 * @brief get a low level interface group.
					 * @param[in] _name Name of the group.
					 * @return Pointer on the requested group or nullptr if the group does not existed.
					 */
					std11::shared_ptr<audio::river::io::Group> getGroup(const std::string& _name);
					
			};
		}
	}
}

#endif

