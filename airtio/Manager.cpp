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

#undef __class__
#define __class__ "Manager"

std::shared_ptr<airtio::Manager> airtio::Manager::create(const std::string& _applicationUniqueId) {
	return std::shared_ptr<airtio::Manager>(new airtio::Manager(_applicationUniqueId));
}

airtio::Manager::Manager(const std::string& _applicationUniqueId) :
  m_applicationUniqueId(_applicationUniqueId),
  m_listOpenInterface(),
  m_masterVolume(0.0f),
  m_masterVolumeRange(std::make_pair(-120.0f, 0.0f)) {
	
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


bool setParameter(const std::string& _flow, const std::string& _filter, const std::string& _parameter, const std::string& _value) {
	AIRTIO_DEBUG("setParameter [BEGIN] : '" << _flow << "':'" << _filter << "':'" << _parameter << "':'" << _value << "'");
	bool out = false;
	if (    _filter == "volume"
	     && _parameter != "FLOW") {
		AIRTIO_ERROR("Interface is not allowed to modify '" << _parameter << "' Volume just allowed to modify 'FLOW' volume");
	}
	AIRTIO_TODO("    IMPLEMENT");
	AIRTIO_DEBUG("setParameter [ END ] : '" << out << "'");
	return out;
}
std::string getParameter(const std::string& _flow, const std::string& _filter, const std::string& _parameter) const {
	AIRTIO_DEBUG("getParameter [BEGIN] : '" << _flow << "':'" << _filter << "':'" << _parameter << "'");
	std::string out;
	AIRTIO_TODO("    IMPLEMENT");
	AIRTIO_DEBUG("getParameter [ END ] : '" << out << "'");
	return out;
}

std::string getParameterProperty(const std::string& _flow, const std::string& _filter, const std::string& _parameter) const {
	AIRTIO_DEBUG("getParameterProperty [BEGIN] : '" << _flow << "':'" << _filter << "':'" << _parameter << "'");
	std::string out;
	AIRTIO_TODO("    IMPLEMENT");
	AIRTIO_DEBUG("getParameterProperty [ END ] : '" << out << "'");
	return out;
}

// TODO : Deprecated ...
void airtio::Manager::setMasterOutputVolume(float _gainDB) {
	if (_gainDB < m_masterVolumeRange.first) {
		//throw std::range_error(std::string(_gainDB) + " is out of bonds : [" + m_masterVolumeRange.first + ".." + m_masterVolumeRange.second + "]");
		return;
	}
	if (_gainDB > m_masterVolumeRange.second) {
		//throw std::range_error(std::string(_gainDB) + " is out of bonds : [" + m_masterVolumeRange.first + ".." + m_masterVolumeRange.second + "]");
		return;
	}
	m_masterVolume = _gainDB;
	for (auto &it : m_listOpenInterface) {
		std::shared_ptr<airtio::Interface> tmpElem = it.lock();
		if (tmpElem == nullptr) {
			continue;
		}
		// TODO : Deprecated ...
		//tmpElem->setMasterVolume(m_masterVolume);
	}
}

float airtio::Manager::getMasterOutputVolume() {
	return m_masterVolume;
}

std::pair<float,float> airtio::Manager::getMasterOutputVolumeRange() {
	return m_masterVolumeRange;
}

void airtio::Manager::setSectionVolume(const std::string& _section, float _gainDB) {
	
}

float airtio::Manager::getSectionVolume(const std::string& _section) {
	return 0.0f;
}

std::pair<float,float> airtio::Manager::getSectionVolumeRange(const std::string& _section) {
	return std::make_pair(0.0f, 0.0f);
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
	std::shared_ptr<airtio::io::Node> node = manager->getNode(_streamName, false);
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
	std::shared_ptr<airtio::io::Node> node = manager->getNode(_streamName, true);
	// create user iterface:
	std::shared_ptr<airtio::Interface> interface;
	interface = airtio::Interface::create(_name, _freq, _map, _format, node);
	// store it in a list (needed to apply some parameters).
	m_listOpenInterface.push_back(interface);
	// TODO : DEPRECATED ...
	//interface->setMasterVolume(m_masterVolume);
	return interface;
}
