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
#include <airtalgo/format.h>
#include <airtalgo/channel.h>
#include <ejson/ejson.h>
#include <memory>
#include <airtalgo/Volume.h>

namespace airtio {
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
				std::vector<std::shared_ptr<airtio::io::Node> > m_listKeepAlive; //!< list of all Node that might be keep alive sone time
				std::vector<std::weak_ptr<airtio::io::Node> > m_list; //!< List of all IO node
			public:
				std::shared_ptr<airtio::io::Node> getNode(const std::string& _name);
			private:
				std::vector<std::shared_ptr<airtalgo::VolumeElement>> m_volumeGroup;
			public:
				std::shared_ptr<airtalgo::VolumeElement> getVolumeGroup(const std::string& _name);
			
				/**
				 * @brief Set a parameter in the stream flow
				 * @param[in] _flow Low level Flow name (see json config file)
				 * @param[in] _filter name of the filter (if you added some personels)
				 * @param[in] _parameter Parameter name.
				 * @param[in] _value Value to set.
				 * @return true set done
				 * @return false An error occured
				 * @example : setParameter("", "volume", "MASTER", "-3dB");
				 * @example : setParameter("microphone", "LowPassFilter", "cutFrequency", "1000Hz");
				 */
				virtual bool setParameter(const std::string& _flow, const std::string& _filter, const std::string& _parameter, const std::string& _value);
				/**
				 * @brief Get a parameter value
				 * @param[in] _flow Low level Flow name (see json config file)
				 * @param[in] _filter name of the filter (if you added some personels)
				 * @param[in] _parameter Parameter name.
				 * @return The requested value.
				 * @example : getParameter("", "volume", "MASTER"); can return something like "-3dB"
				 * @example : getParameter("microphone", "LowPassFilter", "cutFrequency"); can return something like "[-120..0]dB"
				 */
				virtual std::string getParameter(const std::string& _flow, const std::string& _filter, const std::string& _parameter) const;
				/**
				 * @brief Get a parameter value
				 * @param[in] _flow Low level Flow name (see json config file)
				 * @param[in] _filter name of the filter (if you added some personels)
				 * @param[in] _parameter Parameter name.
				 * @return The requested value.
				 * @example : getParameter("", "volume", "MASTER"); can return something like "[-120..0]dB"
				 * @example : getParameter("microphone", "LowPassFilter", "cutFreqiency"); can return something like "]100..10000]Hz"
				 */
				virtual std::string getParameterProperty(const std::string& _flow, const std::string& _filter, const std::string& _parameter) const;
		};
	}
}

#endif

