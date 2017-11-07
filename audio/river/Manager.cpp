/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include "Manager.hpp"
#include "Interface.hpp"
#include "io/Manager.hpp"
#include "io/Node.hpp"
#include "debug.hpp"
#include <ejson/ejson.hpp>

static ethread::Mutex g_mutex;
static etk::Vector<ememory::WeakPtr<audio::river::Manager> > g_listOfAllManager;

ememory::SharedPtr<audio::river::Manager> audio::river::Manager::create(const etk::String& _applicationUniqueId) {
	ethread::UniqueLock lock(g_mutex);
	for (size_t iii=0; iii<g_listOfAllManager.size() ; ++iii) {
		ememory::SharedPtr<audio::river::Manager> tmp = g_listOfAllManager[iii].lock();
		if (tmp == nullptr) {
			continue;
		}
		if (tmp->m_applicationUniqueId == _applicationUniqueId) {
			return tmp;
		}
	}
	// create a new one:
	ememory::SharedPtr<audio::river::Manager> out = ememory::SharedPtr<audio::river::Manager>(ETK_NEW(audio::river::Manager, _applicationUniqueId));
	// add it at the list:
	for (size_t iii=0; iii<g_listOfAllManager.size() ; ++iii) {
		if (g_listOfAllManager[iii].expired() == true) {
			g_listOfAllManager[iii] = out;
			return out;
		}
	}
	g_listOfAllManager.pushBack(out);
	return out;
}

audio::river::Manager::Manager(const etk::String& _applicationUniqueId) :
  m_applicationUniqueId(_applicationUniqueId),
  m_listOpenInterface() {
	
}

audio::river::Manager::~Manager() {
	// TODO : Stop all interfaces...
	
}

etk::Vector<etk::String> audio::river::Manager::getListStreamInput() {
	etk::Vector<etk::String> output;
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
	} else {
		output = manager->getListStreamInput();
	}
	return output;
}

etk::Vector<etk::String> audio::river::Manager::getListStreamOutput() {
	etk::Vector<etk::String> output;
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
	} else {
		output = manager->getListStreamOutput();
	}
	return output;
}

etk::Vector<etk::String> audio::river::Manager::getListStreamVirtual() {
	etk::Vector<etk::String> output;
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
	} else {
		output = manager->getListStreamVirtual();
	}
	return output;
}

etk::Vector<etk::String> audio::river::Manager::getListStream() {
	etk::Vector<etk::String> output;
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
	} else {
		output = manager->getListStream();
	}
	return output;
}

bool audio::river::Manager::setVolume(const etk::String& _volumeName, float _valuedB) {
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return false;
	}
	return manager->setVolume(_volumeName, _valuedB);
}

float audio::river::Manager::getVolume(const etk::String& _volumeName) const {
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return false;
	}
	return manager->getVolume(_volumeName);
}

etk::Pair<float,float> audio::river::Manager::getVolumeRange(const etk::String& _volumeName) const {
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return etk::makePair<float,float>(0.0f,0.0f);
	}
	return manager->getVolumeRange(_volumeName);
}

void audio::river::Manager::setMute(const etk::String& _volumeName, bool _mute) {
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return;
	}
	manager->setMute(_volumeName, _mute);
}

bool audio::river::Manager::getMute(const etk::String& _volumeName) const {
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return false;
	}
	return manager->getMute(_volumeName);
}

ememory::SharedPtr<audio::river::Interface> audio::river::Manager::createOutput(float _freq,
                                                                                const etk::Vector<audio::channel>& _map,
                                                                                audio::format _format,
                                                                                const etk::String& _streamName,
                                                                                const etk::String& _options) {
	// get global hardware interface:
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return ememory::SharedPtr<audio::river::Interface>();
	}
	// get the output or input channel :
	ememory::SharedPtr<audio::river::io::Node> node = manager->getNode(_streamName);
	if (node == nullptr) {
		RIVER_ERROR("Can not get the Requested stream '" << _streamName << "' ==> not listed in : " << manager->getListStream());
		return ememory::SharedPtr<audio::river::Interface>();
	}
	if (node->isOutput() != true) {
		RIVER_ERROR("Can not Connect output on other thing than output ... for stream '" << _streamName << "'");;
		return ememory::SharedPtr<audio::river::Interface>();
	}
	// create user iterface:
	ememory::SharedPtr<audio::river::Interface> interface;
	ejson::Object tmpOption = ejson::Object(_options);
	tmpOption.add("io", ejson::String("output"));
	interface = audio::river::Interface::create(_freq, _map, _format, node, tmpOption);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.pushBack(interface);
	return interface;
}

ememory::SharedPtr<audio::river::Interface> audio::river::Manager::createInput(float _freq,
                                                                               const etk::Vector<audio::channel>& _map,
                                                                               audio::format _format,
                                                                               const etk::String& _streamName,
                                                                               const etk::String& _options) {
	// get global hardware interface:
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return ememory::SharedPtr<audio::river::Interface>();
	}
	// get the output or input channel :
	ememory::SharedPtr<audio::river::io::Node> node = manager->getNode(_streamName);
	if (node == nullptr) {
		RIVER_ERROR("Can not get the Requested stream '" << _streamName << "' ==> not listed in : " << manager->getListStream());
		return ememory::SharedPtr<audio::river::Interface>();
	}
	if (node->isInput() != true) {
		RIVER_ERROR("Can not Connect input on other thing than input ... for stream '" << _streamName << "'");;
		return ememory::SharedPtr<audio::river::Interface>();
	}
	// create user iterface:
	ememory::SharedPtr<audio::river::Interface> interface;
	ejson::Object tmpOption = ejson::Object(_options);
	tmpOption.add("io", ejson::String("input"));
	interface = audio::river::Interface::create(_freq, _map, _format, node, tmpOption);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.pushBack(interface);
	return interface;
}


ememory::SharedPtr<audio::river::Interface> audio::river::Manager::createFeedback(float _freq,
                                                                                  const etk::Vector<audio::channel>& _map,
                                                                                  audio::format _format,
                                                                                  const etk::String& _streamName,
                                                                                  const etk::String& _options) {
	// get global hardware interface:
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Unable to load harware IO manager ... ");
		return ememory::SharedPtr<audio::river::Interface>();
	}
	// get the output or input channel :
	ememory::SharedPtr<audio::river::io::Node> node = manager->getNode(_streamName);
	if (node == nullptr) {
		RIVER_ERROR("Can not get the Requested stream '" << _streamName << "' ==> not listed in : " << manager->getListStream());
		return ememory::SharedPtr<audio::river::Interface>();
	}
	if (node->isOutput() != true) {
		RIVER_ERROR("Can not Connect feedback on other thing than output ... for stream '" << _streamName << "'");;
		return ememory::SharedPtr<audio::river::Interface>();
	}
	// create user iterface:
	ememory::SharedPtr<audio::river::Interface> interface;
	ejson::Object tmpOption = ejson::Object(_options);
	tmpOption.add("io", ejson::String("feedback"));
	interface = audio::river::Interface::create(_freq, _map, _format, node, tmpOption);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.pushBack(interface);
	return interface;
}

void audio::river::Manager::generateDotAll(const etk::String& _filename) {
	// get global hardware interface:
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	if (manager == nullptr) {
		RIVER_ERROR("Can not get the harware manager");
		return;
	}
	manager->generateDot(_filename);
}