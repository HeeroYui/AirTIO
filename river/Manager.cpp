/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "Manager.h"
#include "Interface.h"
#include <memory>
#include <stdexcept>

#include "io/Manager.h"
#include "io/Node.h"
#include "debug.h"

#undef __class__
#define __class__ "Manager"

std::shared_ptr<river::Manager> river::Manager::create(const std::string& _applicationUniqueId) {
	return std::shared_ptr<river::Manager>(new river::Manager(_applicationUniqueId));
}

river::Manager::Manager(const std::string& _applicationUniqueId) :
  m_applicationUniqueId(_applicationUniqueId),
  m_listOpenInterface() {
	
}

river::Manager::~Manager() {
	// TODO : Stop all interfaces...
	
}

std::vector<std::pair<std::string,std::string> > river::Manager::getListStreamInput() {
	std::vector<std::pair<std::string,std::string> > output;
	//output.push_back(std::make_pair<std::string,std::string>("default", "48000 Hz, 16 bits, 2 channels: Default input "));
	return output;
}

std::vector<std::pair<std::string,std::string> > river::Manager::getListStreamOutput() {
	std::vector<std::pair<std::string,std::string> > output;
	//output.push_back(std::make_pair<std::string,std::string>("default", "48000 Hz, 16 bits, 2 channels: Default output "));
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

std::shared_ptr<river::Interface> river::Manager::createOutput(float _freq,
                                                               const std::vector<audio::channel>& _map,
                                                               audio::format _format,
                                                               const std::string& _streamName,
                                                               const std::string& _name) {
	// get global hardware interface:
	std::shared_ptr<river::io::Manager> manager = river::io::Manager::getInstance();
	// get the output or input channel :
	std::shared_ptr<river::io::Node> node = manager->getNode(_streamName);
	// create user iterface:
	std::shared_ptr<river::Interface> interface;
	interface = river::Interface::create(_name, _freq, _map, _format, node, false);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.push_back(interface);
	return interface;
}

std::shared_ptr<river::Interface> river::Manager::createInput(float _freq,
                                                              const std::vector<audio::channel>& _map,
                                                              audio::format _format,
                                                              const std::string& _streamName,
                                                              const std::string& _name) {
	// get global hardware interface:
	std::shared_ptr<river::io::Manager> manager = river::io::Manager::getInstance();
	// get the output or input channel :
	std::shared_ptr<river::io::Node> node = manager->getNode(_streamName);
	// create user iterface:
	std::shared_ptr<river::Interface> interface;
	interface = river::Interface::create(_name, _freq, _map, _format, node, true);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.push_back(interface);
	return interface;
}
