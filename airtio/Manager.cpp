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

std::shared_ptr<airtio::Manager> airtio::Manager::create(const std::string& _applicationUniqueId) {
	return std::shared_ptr<airtio::Manager>(new airtio::Manager(_applicationUniqueId));
}

airtio::Manager::Manager(const std::string& _applicationUniqueId) :
  m_applicationUniqueId(_applicationUniqueId),
  m_listOpenInterface() {
	
}

airtio::Manager::~Manager() {
	// TODO : Stop all interfaces...
	
}

std::vector<std::pair<std::string,std::string> > airtio::Manager::getListStreamInput() {
	std::vector<std::pair<std::string,std::string> > output;
	output.push_back(std::make_pair<std::string,std::string>("default", "48000 Hz, 16 bits, 2 channels: Default input "));
	return output;
}

std::vector<std::pair<std::string,std::string> > airtio::Manager::getListStreamOutput() {
	std::vector<std::pair<std::string,std::string> > output;
	output.push_back(std::make_pair<std::string,std::string>("default", "48000 Hz, 16 bits, 2 channels: Default output "));
	return output;
}


bool airtio::Manager::setParameter(const std::string& _flow, const std::string& _filter, const std::string& _parameter, const std::string& _value) {
	return airtio::io::Manager::getInstance()->setParameter(_flow, _filter, _parameter, _value);
}

std::string airtio::Manager::getParameter(const std::string& _flow, const std::string& _filter, const std::string& _parameter) const {
	return airtio::io::Manager::getInstance()->getParameter(_flow, _filter, _parameter);
}

std::string airtio::Manager::getParameterProperty(const std::string& _flow, const std::string& _filter, const std::string& _parameter) const {
	return airtio::io::Manager::getInstance()->getParameterProperty(_flow, _filter, _parameter);
}

std::shared_ptr<airtio::Interface>
airtio::Manager::createOutput(float _freq,
                                             const std::vector<airtalgo::channel>& _map,
                                             airtalgo::format _format,
                                             const std::string& _streamName,
                                             const std::string& _name) {
	// get global hardware interface:
	std::shared_ptr<airtio::io::Manager> manager = airtio::io::Manager::getInstance();
	// get the output or input channel :
	std::shared_ptr<airtio::io::Node> node = manager->getNode(_streamName);//, false);
	// create user iterface:
	std::shared_ptr<airtio::Interface> interface;
	interface = airtio::Interface::create(_name, _freq, _map, _format, node);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.push_back(interface);
	// TODO : DEPRECATED ...
	//interface->setMasterVolume(m_masterVolume);
	return interface;
}

std::shared_ptr<airtio::Interface>
airtio::Manager::createInput(float _freq,
                                            const std::vector<airtalgo::channel>& _map,
                                            airtalgo::format _format,
                                            const std::string& _streamName,
                                            const std::string& _name) {
	// get global hardware interface:
	std::shared_ptr<airtio::io::Manager> manager = airtio::io::Manager::getInstance();
	// get the output or input channel :
	std::shared_ptr<airtio::io::Node> node = manager->getNode(_streamName);//, true);
	// create user iterface:
	std::shared_ptr<airtio::Interface> interface;
	interface = airtio::Interface::create(_name, _freq, _map, _format, node);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.push_back(interface);
	// TODO : DEPRECATED ...
	//interface->setMasterVolume(m_masterVolume);
	return interface;
}
