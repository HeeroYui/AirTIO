/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "Manager.h"
#include <memory>
#include <airtio/debug.h>
#include "Node.h"

#undef __class__
#define __class__ "io::Manager"

airtio::io::Manager::Manager() {
	if (m_config.load("DATA:hardware.json") == false) {
		AIRTIO_ERROR("you must set a basic configuration file for harware configuration: DATA:hardware.json");
	}
};


std::shared_ptr<airtio::io::Manager> airtio::io::Manager::getInstance() {
	static std::shared_ptr<airtio::io::Manager> manager(new Manager());
	return manager;
}

std::shared_ptr<airtio::io::Node> airtio::io::Manager::getNode(const std::string& _name) {
	for (size_t iii=0; iii< m_list.size(); ++iii) {
		std::shared_ptr<airtio::io::Node> tmppp = m_list[iii].lock();
		if (    tmppp != nullptr
		     && _name == tmppp->getName()) {
			return tmppp;
		}
	}
	// check if the node can be open :
	const std::shared_ptr<const ejson::Object> tmpObject = m_config.getObject(_name);
	if (tmpObject != nullptr) {
		std::shared_ptr<airtio::io::Node> tmp = airtio::io::Node::create(_name, tmpObject);
		m_list.push_back(tmp);
		return tmp;
	}
	AIRTIO_ERROR("Can not create the interface : '" << _name << "' the node is not DEFINED in the configuration file availlable : " << m_config.getKeys());
	return nullptr;
}

std::shared_ptr<airtalgo::VolumeElement> airtio::io::Manager::getVolumeGroup(const std::string& _name) {
	if (_name == "") {
		AIRTIO_ERROR("Try to create an audio group with no name ...");
		return nullptr;
	}
	for (auto &it : m_volumeGroup) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() == _name) {
			return nullptr;
		}
	}
	AIRTIO_DEBUG("Add a new volume group : '" << _name << "'");
	std::shared_ptr<airtalgo::VolumeElement> tmpVolume = std::make_shared<airtalgo::VolumeElement>(_name);
	m_volumeGroup.push_back(tmpVolume);
	return tmpVolume;
}


bool airtio::io::Manager::setParameter(const std::string& _flow, const std::string& _filter, const std::string& _parameter, const std::string& _value) {
	AIRTIO_DEBUG("setParameter [BEGIN] : '" << _flow << "':'" << _filter << "':'" << _parameter << "':'" << _value << "'");
	bool out = false;
	if (    _flow == ""
	     && _filter == "volume") {
		// set IO volume
		float value = 0;
		if (sscanf(_value.c_str(), "%fdB", &value) != 1) {
			AIRTIO_ERROR("Can not parse the value of audio volume : '" << _value << "'");
			return false;
		}
		for (auto &it : m_volumeGroup) {
			if (it == nullptr) {
				continue;
			}
			if (it->getName() == _parameter) {
				it->setVolume(value);
				for (auto &it2 : m_list) {
					std::shared_ptr<airtio::io::Node> val = it2.lock();
					if (val != nullptr) {
						// TODO : notify nodes ...
						// val->
					}
				}
				return true;
			}
		}
	}
	AIRTIO_TODO("    IMPLEMENT");
	AIRTIO_DEBUG("setParameter [ END ] : '" << out << "'");
	return out;
}

std::string airtio::io::Manager::getParameter(const std::string& _flow, const std::string& _filter, const std::string& _parameter) const {
	AIRTIO_DEBUG("getParameter [BEGIN] : '" << _flow << "':'" << _filter << "':'" << _parameter << "'");
	std::string out;
	if (    _flow == ""
	     && _filter == "volume") {
		// get IO volume
		for (auto &it : m_volumeGroup) {
			if (it == nullptr) {
				continue;
			}
			if (it->getName() == _parameter) {
				return std::to_string(it->getVolume()) + "dB";;
			}
		}
	}
	AIRTIO_TODO("    IMPLEMENT");
	AIRTIO_DEBUG("getParameter [ END ] : '" << out << "'");
	return out;
}

std::string airtio::io::Manager::getParameterProperty(const std::string& _flow, const std::string& _filter, const std::string& _parameter) const {
	AIRTIO_DEBUG("getParameterProperty [BEGIN] : '" << _flow << "':'" << _filter << "':'" << _parameter << "'");
	std::string out;
	if (    _flow == ""
	     && _filter == "volume") {
		// return generic volumes
		return "[-300..300]dB";
	}
	AIRTIO_TODO("    IMPLEMENT");
	AIRTIO_DEBUG("getParameterProperty [ END ] : '" << out << "'");
	return out;
}
