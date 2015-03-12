/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_IO_MANAGER_H__
#define __RIVER_IO_MANAGER_H__

#include <string>
#include <vector>
#include <map>
#include <list>
#include <stdint.h>
#include <etk/mutex.h>
#include <etk/chrono.h>
#include <etk/functional.h>
#include <etk/memory.h>
#include <audio/format.h>
#include <audio/channel.h>
#include <ejson/ejson.h>
#include <drain/Volume.h>
#include <river/io/Group.h>

namespace river {
	namespace io {
		class Node;
		class Manager {
			private:
				mutable std11::recursive_mutex m_mutex;
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
				void init(const std::string& _filename);
				void initString(const std::string& _data);
				void unInit();
			private:
				ejson::Document m_config; // harware configuration
				std::vector<std11::shared_ptr<river::io::Node> > m_listKeepAlive; //!< list of all Node that might be keep alive sone time
				std::vector<std11::weak_ptr<river::io::Node> > m_list; //!< List of all IO node
			public:
				std11::shared_ptr<river::io::Node> getNode(const std::string& _name);
			private:
				std::vector<std11::shared_ptr<drain::VolumeElement> > m_volumeGroup;
			public:
				std11::shared_ptr<drain::VolumeElement> getVolumeGroup(const std::string& _name);
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
				 * @brief Generate the dot file corresponding at the actif nodes.
				 * @param[in] _filename Name of the file to write data.
				 */
				void generateDot(const std::string& _filename);
			private:
				std::map<std::string, std11::shared_ptr<river::io::Group> > m_listGroup; //!< List of all groups
				std11::shared_ptr<river::io::Group> getGroup(const std::string& _name);
				
		};
	}
}

#endif

