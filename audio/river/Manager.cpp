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
#include <ejson/ejson.h>

static std::mutex g_mutex;
static std::vector<std::weak_ptr<audio::river::Manager> > g_listOfAllManager;

std::shared_ptr<audio::river::Manager> audio::river::Manager::create(const std::string& _applicationUniqueId) {
	std::unique_lock<std::mutex> lock(g_mutex);
	for (size_t iii=0; iii<g_listOfAllManager.size() ; ++iii) {
		std::shared_ptr<audio::river::Manager> tmp = g_listOfAllManager[iii].lock();
		if (tmp == nullptr) {
			continue;
		}
		if (tmp->m_applicationUniqueId == _applicationUniqueId) {
			return tmp;
		}
	}
	// create a new one:
	std::shared_ptr<audio::river::Manager> out = std::shared_ptr<audio::river::Manager>(new audio::river::Manager(_applicationUniqueId));
	// add it at the list:
	for (size_t iii=0; iii<g_listOfAllManager.size() ; ++iii) {
		if (g_listOfAllManager[iii].expired() == true) {
			g_listOfAllManager[iii] = out;
			return out;
		}
	}
	g_listOfAllManager.push_back(out);
	return out;
}

audio::river::Manager::Manager(const std::string& _applicationUniqueId) :
  m_applicationUniqueId(_applicationUniqueId),
  m_listOpenInterface() {
	
}

audio::river::Manager::~Manager() {
	// TODO : Stop all interfaces...
	
}

std::vector<std::string> audio::river::Manager::getListStreamInput() {
	std::vector<std::string> output;
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
	} else {
		output = manager->getListStreamInput();
	}
	return output;
}

std::vector<std::string> audio::river::Manager::getListStreamOutput() {
	std::vector<std::string> output;
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
	} else {
		output = manager->getListStreamOutput();
	}
	return output;
}

std::vector<std::string> audio::river::Manager::getListStreamVirtual() {
	std::vector<std::string> output;
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
	} else {
		output = manager->getListStreamVirtual();
	}
	return output;
}

std::vector<std::string> audio::river::Manager::getListStream() {
	std::vector<std::string> output;
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
	} else {
		output = manager->getListStream();
	}
	return output;
}

bool audio::river::Manager::setVolume(const std::string& _volumeName, float _valuedB) {
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return false;
	}
	return manager->setVolume(_volumeName, _valuedB);
}

float audio::river::Manager::getVolume(const std::string& _volumeName) const {
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return false;
	}
	return manager->getVolume(_volumeName);
}

std::pair<float,float> audio::river::Manager::getVolumeRange(const std::string& _volumeName) const {
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return std::make_pair<float,float>(0.0f,0.0f);
	}
	return manager->getVolumeRange(_volumeName);
}

void audio::river::Manager::setMute(const std::string& _volumeName, bool _mute) {
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return;
	}
	manager->setMute(_volumeName, _mute);
}

bool audio::river::Manager::getMute(const std::string& _volumeName) const {
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return false;
	}
	return manager->getMute(_volumeName);
}

std::shared_ptr<audio::river::Interface> audio::river::Manager::createOutput(float _freq,
                                                                             const std::vector<audio::channel>& _map,
                                                                             audio::format _format,
                                                                             const std::string& _streamName,
                                                                             const std::string& _options) {
	// get global hardware interface:
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return std::shared_ptr<audio::river::Interface>();
	}
	// get the output or input channel :
	std::shared_ptr<audio::river::io::Node> node = manager->getNode(_streamName);
	if (node == nullptr) {
		RIVER_ERROR("Can not get the Requested stream '" << _streamName << "' ==> not listed in : " << manager->getListStream());
		return std::shared_ptr<audio::river::Interface>();
	}
	if (node->isOutput() != true) {
		RIVER_ERROR("Can not Connect output on other thing than output ... for stream '" << _streamName << "'");;
		return std::shared_ptr<audio::river::Interface>();
	}
	// create user iterface:
	std::shared_ptr<audio::river::Interface> interface;
	ejson::Object tmpOption = ejson::Object(_options);
	tmpOption.add("io", ejson::String("output"));
	interface = audio::river::Interface::create(_freq, _map, _format, node, tmpOption);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.push_back(interface);
	return interface;
}

std::shared_ptr<audio::river::Interface> audio::river::Manager::createInput(float _freq,
                                                                              const std::vector<audio::channel>& _map,
                                                                              audio::format _format,
                                                                              const std::string& _streamName,
                                                                              const std::string& _options) {
	// get global hardware interface:
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return std::shared_ptr<audio::river::Interface>();
	}
	// get the output or input channel :
	std::shared_ptr<audio::river::io::Node> node = manager->getNode(_streamName);
	if (node == nullptr) {
		RIVER_ERROR("Can not get the Requested stream '" << _streamName << "' ==> not listed in : " << manager->getListStream());
		return std::shared_ptr<audio::river::Interface>();
	}
	if (node->isInput() != true) {
		RIVER_ERROR("Can not Connect input on other thing than input ... for stream '" << _streamName << "'");;
		return std::shared_ptr<audio::river::Interface>();
	}
	// create user iterface:
	std::shared_ptr<audio::river::Interface> interface;
	ejson::Object tmpOption = ejson::Object(_options);
	tmpOption.add("io", ejson::String("input"));
	interface = audio::river::Interface::create(_freq, _map, _format, node, tmpOption);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.push_back(interface);
	return interface;
}


std::shared_ptr<audio::river::Interface> audio::river::Manager::createFeedback(float _freq,
                                                                                 const std::vector<audio::channel>& _map,
                                                                                 audio::format _format,
                                                                                 const std::string& _streamName,
                                                                                 const std::string& _options) {
	// get global hardware interface:
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return std::shared_ptr<audio::river::Interface>();
	}
	// get the output or input channel :
	std::shared_ptr<audio::river::io::Node> node = manager->getNode(_streamName);
	if (node == nullptr) {
		RIVER_ERROR("Can not get the Requested stream '" << _streamName << "' ==> not listed in : " << manager->getListStream());
		return std::shared_ptr<audio::river::Interface>();
	}
	if (node->isOutput() != true) {
		RIVER_ERROR("Can not Connect feedback on other thing than output ... for stream '" << _streamName << "'");;
		return std::shared_ptr<audio::river::Interface>();
	}
	// create user iterface:
	std::shared_ptr<audio::river::Interface> interface;
	ejson::Object tmpOption = ejson::Object(_options);
	tmpOption.add("io", ejson::String("feedback"));
	interface = audio::river::Interface::create(_freq, _map, _format, node, tmpOption);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.push_back(interface);
	return interface;
}

void audio::river::Manager::generateDotAll(const std::string& _filename) {
	// get global hardware interface:
	std::shared_ptr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Can not get the harware manager");
		return;
	}
	manager->generateDot(_filename);
}