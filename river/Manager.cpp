/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "Manager.h"
#include "Interface.h"
#include <stdexcept>

#include "io/Manager.h"
#include "io/Node.h"
#include "debug.h"

#undef __class__
#define __class__ "Manager"

static std::string basicAutoConfig = 
	"{\n"
	"	microphone:{\n"
	"		io:'input',\n"
	"		map-on:'microphone',\n"
	"		resampling-type:'speexdsp',\n"
	"		resampling-option:'quality=10'\n"
	"	},\n"
	"	speaker:{\n"
	"		io:'output',\n"
	"		map-on:'speaker',\n"
	"		resampling-type:'speexdsp',\n"
	"		resampling-option:'quality=10'\n"
	"	}\n"
	"}\n";


std11::shared_ptr<river::Manager> river::Manager::create(const std::string& _applicationUniqueId) {
	return std11::shared_ptr<river::Manager>(new river::Manager(_applicationUniqueId));
}

river::Manager::Manager(const std::string& _applicationUniqueId) :
  m_applicationUniqueId(_applicationUniqueId),
  m_listOpenInterface() {
	// TODO : Maybe create a single interface property (and all get the same ...)
	if (m_config.load("DATA:virtual.json") == false) {
		RIVER_WARNING("you must set a basic configuration file for virtual configuration: DATA:virtual.json (load default interface)");
		m_config.parse(basicAutoConfig);
	}
}

river::Manager::~Manager() {
	// TODO : Stop all interfaces...
	
}

std::vector<std::pair<std::string,std::string> > river::Manager::getListStreamInput() {
	std::vector<std::pair<std::string,std::string> > output;
	std::vector<std::string> keys = m_config.getKeys();
	for (size_t iii=0; iii<keys.size(); ++iii) {
		const std11::shared_ptr<const ejson::Object> tmppp = m_config.getObject(keys[iii]);
		if (tmppp != nullptr) {
			std::string type = tmppp->getStringValue("io", "error");
			if (    type == "input"
			     || type == "feedback") {
				output.push_back(std::make_pair<std::string,std::string>(std::string(keys[iii]), std::string("---")));
			}
		}
	}
	return output;
}

std::vector<std::pair<std::string,std::string> > river::Manager::getListStreamOutput() {
	std::vector<std::pair<std::string,std::string> > output;
	std::vector<std::string> keys = m_config.getKeys();
	for (size_t iii=0; iii<keys.size(); ++iii) {
		const std11::shared_ptr<const ejson::Object> tmppp = m_config.getObject(keys[iii]);
		if (tmppp != nullptr) {
			std::string type = tmppp->getStringValue("io", "error");
			if (type == "output") {
				output.push_back(std::make_pair<std::string,std::string>(std::string(keys[iii]), std::string("---")));
			}
		}
	}
	return output;
}


bool river::Manager::setVolume(const std::string& _volumeName, float _valuedB) {
	return river::io::Manager::getInstance()->setVolume(_volumeName, _valuedB);
}

float river::Manager::getVolume(const std::string& _volumeName) const {
	return river::io::Manager::getInstance()->getVolume(_volumeName);
}

std::pair<float,float> river::Manager::getVolumeRange(const std::string& _volumeName) const {
	return river::io::Manager::getInstance()->getVolumeRange(_volumeName);
}

std11::shared_ptr<river::Interface> river::Manager::createOutput(float _freq,
                                                               const std::vector<audio::channel>& _map,
                                                               audio::format _format,
                                                               const std::string& _streamName,
                                                               const std::string& _name) {
	// check if the output exist
	const std11::shared_ptr<const ejson::Object> tmppp = m_config.getObject(_streamName);
	if (tmppp == nullptr) {
		RIVER_ERROR("can not open a non existance virtual input: '" << _streamName << "' not present in : " << m_config.getKeys());
		return std11::shared_ptr<river::Interface>();
	}
	// check if it is an Output:
	std::string type = tmppp->getStringValue("io", "error");
	if (type != "output") {
		RIVER_ERROR("can not open in output a virtual interface: '" << _streamName << "' configured has : " << type);
		return std11::shared_ptr<river::Interface>();
	}
	std::string mapOn = tmppp->getStringValue("map-on", "");
	if (mapOn == "") {
		RIVER_ERROR("can not open in output a virtual interface: '" << _streamName << "' No 'map-on' element in json file ... ");
		return std11::shared_ptr<river::Interface>();
	}
	// get global hardware interface:
	std11::shared_ptr<river::io::Manager> manager = river::io::Manager::getInstance();
	// get the output or input channel :
	std11::shared_ptr<river::io::Node> node = manager->getNode(mapOn);
	// create user iterface:
	std11::shared_ptr<river::Interface> interface;
	interface = river::Interface::create(_name, _freq, _map, _format, node, tmppp);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.push_back(interface);
	return interface;
}

std11::shared_ptr<river::Interface> river::Manager::createInput(float _freq,
                                                              const std::vector<audio::channel>& _map,
                                                              audio::format _format,
                                                              const std::string& _streamName,
                                                              const std::string& _name) {
	// check if the output exist
	const std11::shared_ptr<const ejson::Object> tmppp = m_config.getObject(_streamName);
	if (tmppp == nullptr) {
		RIVER_ERROR("can not open a non existance virtual interface: '" << _streamName << "' not present in : " << m_config.getKeys());
		return std11::shared_ptr<river::Interface>();
	}
	// check if it is an Output:
	std::string type = tmppp->getStringValue("io", "error");
	if (    type != "input"
	     && type != "feedback") {
		RIVER_ERROR("can not open in output a virtual interface: '" << _streamName << "' configured has : " << type);
		return std11::shared_ptr<river::Interface>();
	}
	std::string mapOn = tmppp->getStringValue("map-on", "");
	if (mapOn == "") {
		RIVER_ERROR("can not open in output a virtual interface: '" << _streamName << "' No 'map-on' element in json file ... ");
		return std11::shared_ptr<river::Interface>();
	}
	// get global hardware interface:
	std11::shared_ptr<river::io::Manager> manager = river::io::Manager::getInstance();
	// get the output or input channel :
	std11::shared_ptr<river::io::Node> node = manager->getNode(mapOn);
	// create user iterface:
	std11::shared_ptr<river::Interface> interface;
	interface = river::Interface::create(_name, _freq, _map, _format, node, tmppp);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.push_back(interface);
	return interface;
}

void river::Manager::generateDotAll(const std::string& _filename) {
	// get global hardware interface:
	std11::shared_ptr<river::io::Manager> manager = river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Can not get the harware manager");
		return;
	}
	manager->generateDot(_filename);
}