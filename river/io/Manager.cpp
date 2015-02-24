/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "Manager.h"
#include <river/debug.h>
#include "Node.h"
#include "NodeAEC.h"
#include "NodeAirTAudio.h"
#include "NodePortAudio.h"
#include <etk/os/FSNode.h>

#undef __class__
#define __class__ "io::Manager"

#ifdef __PORTAUDIO_INFERFACE__
	#include <portaudio.h>
#endif

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
	
	#ifdef __PORTAUDIO_INFERFACE__
	PaError err = Pa_Initialize();
	if(err != paNoError) {
		RIVER_WARNING("Can not initialize portaudio : " << Pa_GetErrorText(err));
	}
	#endif
};

river::io::Manager::~Manager() {
	#ifdef __PORTAUDIO_INFERFACE__
	PaError err = Pa_Terminate();
	if(err != paNoError) {
		RIVER_WARNING("Can not initialize portaudio : " << Pa_GetErrorText(err));
	}
	#endif
};


std11::shared_ptr<river::io::Manager> river::io::Manager::getInstance() {
	static std11::shared_ptr<river::io::Manager> manager(new Manager());
	return manager;
}

std11::shared_ptr<river::io::Node> river::io::Manager::getNode(const std::string& _name) {
	RIVER_WARNING("Get node : " << _name);
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		std11::shared_ptr<river::io::Node> tmppp = m_list[iii].lock();
		if (    tmppp != nullptr
		     && _name == tmppp->getName()) {
			RIVER_WARNING(" find it ... ");
			return tmppp;
		}
	}
	RIVER_WARNING("Create a new one : " << _name);
	// check if the node can be open :
	const std11::shared_ptr<const ejson::Object> tmpObject = m_config.getObject(_name);
	if (tmpObject != nullptr) {
		// get type : io
		std::string ioType = tmpObject->getStringValue("io", "error");
		#ifdef __AIRTAUDIO_INFERFACE__
		if (    ioType == "input"
		     || ioType == "output") {
			std11::shared_ptr<river::io::Node> tmp = river::io::NodeAirTAudio::create(_name, tmpObject);
			m_list.push_back(tmp);
			return tmp;
		} else
		#endif
		#ifdef __PORTAUDIO_INFERFACE__
		if (    ioType == "PAinput"
		     || ioType == "PAoutput") {
			std11::shared_ptr<river::io::Node> tmp = river::io::NodePortAudio::create(_name, tmpObject);
			m_list.push_back(tmp);
			return tmp;
		} else 
		#endif
		if (ioType == "aec") {
			std11::shared_ptr<river::io::Node> tmp = river::io::NodeAEC::create(_name, tmpObject);
			m_list.push_back(tmp);
			return tmp;
		}
	}
	RIVER_ERROR("Can not create the interface : '" << _name << "' the node is not DEFINED in the configuration file availlable : " << m_config.getKeys());
	return std11::shared_ptr<river::io::Node>();
}

std11::shared_ptr<drain::VolumeElement> river::io::Manager::getVolumeGroup(const std::string& _name) {
	if (_name == "") {
		RIVER_ERROR("Try to create an audio group with no name ...");
		return std11::shared_ptr<drain::VolumeElement>();
	}
	for (size_t iii=0; iii<m_volumeGroup.size(); ++iii) {
		if (m_volumeGroup[iii] == nullptr) {
			continue;
		}
		if (m_volumeGroup[iii]->getName() == _name) {
			return m_volumeGroup[iii];
		}
	}
	RIVER_DEBUG("Add a new volume group : '" << _name << "'");
	std11::shared_ptr<drain::VolumeElement> tmpVolume = std11::make_shared<drain::VolumeElement>(_name);
	m_volumeGroup.push_back(tmpVolume);
	return tmpVolume;
}

bool river::io::Manager::setVolume(const std::string& _volumeName, float _valuedB) {
	std11::shared_ptr<drain::VolumeElement> volume = getVolumeGroup(_volumeName);
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
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		std11::shared_ptr<river::io::Node> val = m_list[iii].lock();
		if (val != nullptr) {
			val->volumeChange();
		}
	}
	return true;
}

float river::io::Manager::getVolume(const std::string& _volumeName) {
	std11::shared_ptr<drain::VolumeElement> volume = getVolumeGroup(_volumeName);
	if (volume == nullptr) {
		RIVER_ERROR("Can not get volume ... : '" << _volumeName << "'");
		return 0.0f;
	}
	return volume->getVolume();
}

std::pair<float,float> river::io::Manager::getVolumeRange(const std::string& _volumeName) const {
	return std::make_pair<float,float>(-300, 300);
}

void river::io::Manager::generateDot(const std::string& _filename) {
	etk::FSNode node(_filename);
	RIVER_INFO("Generate the DOT files: " << node);
	if (node.fileOpenWrite() == false) {
		RIVER_ERROR("Can not Write the dot file (fail to open) : " << node);
		return;
	}
	node << "digraph G {" << "\n";
	node << "	rankdir=\"LR\";\n";
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		std11::shared_ptr<river::io::Node> val = m_list[iii].lock();
		if (val != nullptr) {
			val->generateDot(node);
		}
	}
	node << "}" << "\n";
	node.fileClose();
	RIVER_INFO("Generate the DOT files: " << node << " (DONE)");
}
