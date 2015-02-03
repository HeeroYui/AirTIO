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