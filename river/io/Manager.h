/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __AIRTIO_IO_MANAGER_H__
#define __AIRTIO_IO_MANAGER_H__

#include <string>
#include <vector>
#include <list>
#include <stdint.h>
#include <chrono>
#include <functional>
#include <audio/format.h>
#include <audio/channel.h>
#include <ejson/ejson.h>
#include <memory>
#include <drain/Volume.h>

namespace river {
	namespace io {
		class Node;
		class Manager {
			private:
				/**
				 * @brief Constructor
				 */
				Manager();
			public:
				static std::shared_ptr<Manager> getInstance();
				/**
				 * @brief Destructor
				 */
				virtual ~Manager() {};
			private:
				ejson::Document m_config; // harware configuration
				std::vector<std::shared_ptr<river::io::Node> > m_listKeepAlive; //!< list of all Node that might be keep alive sone time
				std::vector<std::weak_ptr<river::io::Node> > m_list; //!< List of all IO node
			public:
				std::shared_ptr<river::io::Node> getNode(const std::string& _name);
			private:
				std::vector<std::shared_ptr<drain::VolumeElement>> m_volumeGroup;
			public:
				std::shared_ptr<drain::VolumeElement> getVolumeGroup(const std::string& _name);
				
				/**
				 * @brief Set a volume for a specific group
				 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
				 * @param[in] _value Volume in dB to set.
				 * @return true set done
				 * @return false An error occured
				 * @example : setVolume("MASTER", -3.0f);
				 */
				virtual bool setVolume(const std::string& _volumeName, float _valuedB);
				/**
				 * @brief Get a volume value
				 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
				 * @return The Volume value in dB.
				 * @example ret = getVolume("MASTER"); can return something like ret = -3.0f
				 */
				virtual float getVolume(const std::string& _volumeName);
				/**
				 * @brief Get a parameter value
				 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
				 * @return The requested value Range.
				 * @example ret = getVolumeRange("MASTER"); can return something like ret=(-120.0f,0.0f)
				 */
				virtual std::pair<float,float> getVolumeRange(const std::string& _volumeName) const;
		};
	}
}

#endif

