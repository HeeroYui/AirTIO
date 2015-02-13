/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "Manager.h"
#include <memory>
#include <river/debug.h>
#include "Node.h"

#undef __class__
#define __class__ "io::Manager"


static std::string basicAutoConfig =
	"{\n"
	"	microphone:{\n"
	"		io:'input',\n"
	"		map-on:{\n"
	"			interface:'auto',\n"
	"			name:'default',\n"
	"		},\n"
	"		frequency:0,\n"
	"		channel-map:[\n"
	"			'front-left', 'front-right'\n"
	"		],\n"
	"		type:'auto',\n"
	"		nb-chunk:1024\n"
	"	},\n"
	"	speaker:{\n"
	"		io:'output',\n"
	"		map-on:{\n"
	"			interface:'auto',\n"
	"			name:'default',\n"
	"		},\n"
	"		frequency:0,\n"
	"		channel-map:[\n"
	"			'front-left', 'front-right',\n"
	"		],\n"
	"		type:'auto',\n"
	"		nb-chunk:1024,\n"
	"		volume-name:'MASTER'\n"
	"	}\n"
	"}\n";



river::io::Manager::Manager() {
	if (m_config.load("DATA:hardware.json") == false) {
		RIVER_WARNING("you must set a basic configuration file for harware configuration: DATA:hardware.json (load default interface)");
		m_config.parse(basicAutoConfig);
	}
	// TODO : Load virtual.json and check if all is correct ...
};


std::shared_ptr<river::io::Manager> river::io::Manager::getInstance() {
	static std::shared_ptr<river::io::Manager> manager(new Manager());
	return manager;
}

std::shared_ptr<river::io::Node> river::io::Manager::getNode(const std::string& _name) {
	for (auto &it : m_list) {
		std::shared_ptr<river::io::Node> tmppp = it.lock();
		if (    tmppp != nullptr
		     && _name == tmppp->getName()) {
			return tmppp;
		}
	}
	// check if the node can be open :
	const std::shared_ptr<const ejson::Object> tmpObject = m_config.getObject(_name);
	if (tmpObject != nullptr) {
		std::shared_ptr<river::io::Node> tmp = river::io::Node::create(_name, tmpObject);
		m_list.push_back(tmp);
		return tmp;
	}
	RIVER_ERROR("Can not create the interface : '" << _name << "' the node is not DEFINED in the configuration file availlable : " << m_config.getKeys());
	return nullptr;
}

std::shared_ptr<drain::VolumeElement> river::io::Manager::getVolumeGroup(const std::string& _name) {
	if (_name == "") {
		RIVER_ERROR("Try to create an audio group with no name ...");
		return nullptr;
	}
	for (auto &it : m_volumeGroup) {
		if (it == nullptr) {
			continue;
		}
		if (it->getName() == _name) {
			return it;
		}
	}
	RIVER_DEBUG("Add a new volume group : '" << _name << "'");
	std::shared_ptr<drain::VolumeElement> tmpVolume = std::make_shared<drain::VolumeElement>(_name);
	m_volumeGroup.push_back(tmpVolume);
	return tmpVolume;
}

bool river::io::Manager::setVolume(const std::string& _volumeName, float _valuedB) {
	std::shared_ptr<drain::VolumeElement> volume = getVolumeGroup(_volumeName);
	if (volume == nullptr) {
		RIVER_ERROR("Can not set volume ... : '" << _volumeName << "'");
		return false;
	}
	if (    _valuedB < -300
	     || _valuedB > 300) {
		RIVER_ERROR("Can not set volume ... : '" << _volumeName << "' out of range : [-300..300]");
		return false;
	}
	volume->setVolume(_valuedB);
	for (auto &it2 : m_list) {
		std::shared_ptr<river::io::Node> val = it2.lock();
		if (val != nullptr) {
			val->volumeChange();
		}
	}
	return true;
}

float river::io::Manager::getVolume(const std::string& _volumeName) {
	std::shared_ptr<drain::VolumeElement> volume = getVolumeGroup(_volumeName);
	if (volume == nullptr) {
		RIVER_ERROR("Can not get volume ... : '" << _volumeName << "'");
		return 0.0f;
	}
	return volume->getVolume();
}

std::pair<float,float> river::io::Manager::getVolumeRange(const std::string& _volumeName) const {
	return std::make_pair<float,float>(-300, 300);
}
