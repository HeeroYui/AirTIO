/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <audio/river/io/Manager.h>
#include <audio/river/debug.h>
#include <audio/river/river.h>
#include <audio/river/io/Node.h>
#include <audio/river/io/NodeAEC.h>
#include <audio/river/io/NodeMuxer.h>
#include <audio/river/io/NodeOrchestra.h>
#include <audio/river/io/NodePortAudio.h>
#include <etk/os/FSNode.h>
#include <etk/memory.h>
#include <etk/types.h>
#include <utility>

#undef __class__
#define __class__ "io::Manager"

#ifdef AUDIO_RIVER_BUILD_PORTAUDIO
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



audio::river::io::Manager::Manager() {
	#ifdef AUDIO_RIVER_BUILD_PORTAUDIO
	PaError err = Pa_Initialize();
	if(err != paNoError) {
		RIVER_WARNING("Can not initialize portaudio : " << Pa_GetErrorText(err));
	}
	#endif
}

void audio::river::io::Manager::init(const std::string& _filename) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	if (_filename == "") {
		RIVER_INFO("Load default config");
		m_config.parse(basicAutoConfig);
	} else if (m_config.load(_filename) == false) {
		RIVER_ERROR("you must set a basic configuration file for harware configuration: '" << _filename << "'");
	}
}

void audio::river::io::Manager::initString(const std::string& _data) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	m_config.parse(_data);
}

void audio::river::io::Manager::unInit() {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	// TODO : ...
}

audio::river::io::Manager::~Manager() {
	#ifdef AUDIO_RIVER_BUILD_PORTAUDIO
	PaError err = Pa_Terminate();
	if(err != paNoError) {
		RIVER_WARNING("Can not initialize portaudio : " << Pa_GetErrorText(err));
	}
	#endif
};

std11::shared_ptr<audio::river::io::Manager> audio::river::io::Manager::getInstance() {
	if (audio::river::isInit() == false) {
		return std11::shared_ptr<audio::river::io::Manager>();
	}
	static std11::shared_ptr<audio::river::io::Manager> manager(new Manager());
	return manager;
}


std::vector<std::string> audio::river::io::Manager::getListStreamInput() {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	std::vector<std::string> output;
	std::vector<std::string> keys = m_config.getKeys();
	for (size_t iii=0; iii<keys.size(); ++iii) {
		const std11::shared_ptr<const ejson::Object> tmppp = m_config.getObject(keys[iii]);
		if (tmppp != nullptr) {
			std::string type = tmppp->getStringValue("io", "error");
			if (    type == "input"
			     || type == "PAinput") {
				output.push_back(keys[iii]);
			}
		}
	}
	return output;
}

std::vector<std::string> audio::river::io::Manager::getListStreamOutput() {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	std::vector<std::string> output;
	std::vector<std::string> keys = m_config.getKeys();
	for (size_t iii=0; iii<keys.size(); ++iii) {
		const std11::shared_ptr<const ejson::Object> tmppp = m_config.getObject(keys[iii]);
		if (tmppp != nullptr) {
			std::string type = tmppp->getStringValue("io", "error");
			if (    type == "output"
			     || type == "PAoutput") {
				output.push_back(keys[iii]);
			}
		}
	}
	return output;
}

std::vector<std::string> audio::river::io::Manager::getListStreamVirtual() {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	std::vector<std::string> output;
	std::vector<std::string> keys = m_config.getKeys();
	for (size_t iii=0; iii<keys.size(); ++iii) {
		const std11::shared_ptr<const ejson::Object> tmppp = m_config.getObject(keys[iii]);
		if (tmppp != nullptr) {
			std::string type = tmppp->getStringValue("io", "error");
			if (    type != "input"
			     && type != "PAinput"
			     && type != "output"
			     && type != "PAoutput"
			     && type != "error") {
				output.push_back(keys[iii]);
			}
		}
	}
	return output;
}

std::vector<std::string> audio::river::io::Manager::getListStream() {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	std::vector<std::string> output;
	std::vector<std::string> keys = m_config.getKeys();
	for (size_t iii=0; iii<keys.size(); ++iii) {
		const std11::shared_ptr<const ejson::Object> tmppp = m_config.getObject(keys[iii]);
		if (tmppp != nullptr) {
			std::string type = tmppp->getStringValue("io", "error");
			if (type != "error") {
				output.push_back(keys[iii]);
			}
		}
	}
	return output;
}

std11::shared_ptr<audio::river::io::Node> audio::river::io::Manager::getNode(const std::string& _name) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	RIVER_WARNING("Get node : " << _name);
	// search in the standalone list :
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		std11::shared_ptr<audio::river::io::Node> tmppp = m_list[iii].lock();
		if (    tmppp != nullptr
		     && _name == tmppp->getName()) {
			RIVER_WARNING(" find it ... in standalone");
			return tmppp;
		}
	}
	// search in the group list:
	{
		for (std::map<std::string, std11::shared_ptr<audio::river::io::Group> >::iterator it(m_listGroup.begin());
		     it != m_listGroup.end();
		     ++it) {
			if (it->second != nullptr) {
				std11::shared_ptr<audio::river::io::Node> node = it->second->getNode(_name);
				if (node != nullptr) {
					RIVER_WARNING(" find it ... in group: " << it->first);
					return node;
				}
			}
		}
	}
	RIVER_WARNING("Try create a new one : " << _name);
	// check if the node can be open :
	const std11::shared_ptr<const ejson::Object> tmpObject = m_config.getObject(_name);
	if (tmpObject != nullptr) {
		//Check if it is in a group:
		std::string groupName = tmpObject->getStringValue("group", "");
		// get type : io
		std::string ioType = tmpObject->getStringValue("io", "error");
		if (    groupName != ""
		     && (    ioType == "input"
		          || ioType == "output"
		          || ioType == "PAinput"
		          || ioType == "PAoutput") ) {
			std11::shared_ptr<audio::river::io::Group> tmpGroup = getGroup(groupName);
			if (tmpGroup == nullptr) {
				RIVER_WARNING("Can not get group ... '" << groupName << "'");
				return std11::shared_ptr<audio::river::io::Node>();
			}
			return tmpGroup->getNode(_name);
		} else {
			if (groupName != "") {
				RIVER_WARNING("Group is only availlable for Hardware interface ... '" << _name << "'");
			}
			// TODO : Create a standalone group for every single element ==> simplify understanding ... but not for virtual interface ...
			
			if (    ioType == "input"
			     || ioType == "output") {
				#ifdef AUDIO_RIVER_BUILD_ORCHESTRA
					std11::shared_ptr<audio::river::io::Node> tmp = audio::river::io::NodeOrchestra::create(_name, tmpObject);
					m_list.push_back(tmp);
					return tmp;
				#else
					RIVER_WARNING("not present interface");
				#endif
			}
			if (    ioType == "PAinput"
			     || ioType == "PAoutput") {
				#ifdef AUDIO_RIVER_BUILD_PORTAUDIO
					std11::shared_ptr<audio::river::io::Node> tmp = audio::river::io::NodePortAudio::create(_name, tmpObject);
					m_list.push_back(tmp);
					return tmp;
				#else
					RIVER_WARNING("not present interface");
				#endif
			}
			if (ioType == "aec") {
				std11::shared_ptr<audio::river::io::Node> tmp = audio::river::io::NodeAEC::create(_name, tmpObject);
				m_list.push_back(tmp);
				return tmp;
			}
			if (ioType == "muxer") {
				std11::shared_ptr<audio::river::io::Node> tmp = audio::river::io::NodeMuxer::create(_name, tmpObject);
				m_list.push_back(tmp);
				return tmp;
			}
		}
	}
	RIVER_ERROR("Can not create the interface : '" << _name << "' the node is not DEFINED in the configuration file availlable : " << m_config.getKeys());
	return std11::shared_ptr<audio::river::io::Node>();
}

std11::shared_ptr<audio::drain::VolumeElement> audio::river::io::Manager::getVolumeGroup(const std::string& _name) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	if (_name == "") {
		RIVER_ERROR("Try to create an audio group with no name ...");
		return std11::shared_ptr<audio::drain::VolumeElement>();
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
	std11::shared_ptr<audio::drain::VolumeElement> tmpVolume = std11::make_shared<audio::drain::VolumeElement>(_name);
	m_volumeGroup.push_back(tmpVolume);
	return tmpVolume;
}

bool audio::river::io::Manager::setVolume(const std::string& _volumeName, float _valuedB) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	std11::shared_ptr<audio::drain::VolumeElement> volume = getVolumeGroup(_volumeName);
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
		std11::shared_ptr<audio::river::io::Node> val = m_list[iii].lock();
		if (val != nullptr) {
			val->volumeChange();
		}
	}
	return true;
}

float audio::river::io::Manager::getVolume(const std::string& _volumeName) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	std11::shared_ptr<audio::drain::VolumeElement> volume = getVolumeGroup(_volumeName);
	if (volume == nullptr) {
		RIVER_ERROR("Can not get volume ... : '" << _volumeName << "'");
		return 0.0f;
	}
	return volume->getVolume();
}

std::pair<float,float> audio::river::io::Manager::getVolumeRange(const std::string& _volumeName) const {
	return std::make_pair<float,float>(-300, 300);
}

void audio::river::io::Manager::generateDot(const std::string& _filename) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	etk::FSNode node(_filename);
	RIVER_INFO("Generate the DOT files: " << node);
	if (node.fileOpenWrite() == false) {
		RIVER_ERROR("Can not Write the dot file (fail to open) : " << node);
		return;
	}
	node << "digraph G {" << "\n";
	node << "	rankdir=\"LR\";\n";
	// First Step : Create all HW interface:
	{
		// standalone
		for (size_t iii=0; iii<m_list.size(); ++iii) {
			std11::shared_ptr<audio::river::io::Node> val = m_list[iii].lock();
			if (val != nullptr) {
				if (val->isHarwareNode() == true) {
					val->generateDot(node);
				}
			}
		}
		for (std::map<std::string, std11::shared_ptr<audio::river::io::Group> >::iterator it(m_listGroup.begin());
		     it != m_listGroup.end();
		     ++it) {
			if (it->second != nullptr) {
				it->second->generateDot(node, true);
			}
		}
	}
	// All other ...
	{
		// standalone
		for (size_t iii=0; iii<m_list.size(); ++iii) {
			std11::shared_ptr<audio::river::io::Node> val = m_list[iii].lock();
			if (val != nullptr) {
				if (val->isHarwareNode() == false) {
					val->generateDot(node);
				}
			}
		}
		for (std::map<std::string, std11::shared_ptr<audio::river::io::Group> >::iterator it(m_listGroup.begin());
		     it != m_listGroup.end();
		     ++it) {
			if (it->second != nullptr) {
				it->second->generateDot(node, false);
			}
		}
	}
	
	node << "}" << "\n";
	node.fileClose();
	RIVER_INFO("Generate the DOT files: " << node << " (DONE)");
}

std11::shared_ptr<audio::river::io::Group> audio::river::io::Manager::getGroup(const std::string& _name) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	std11::shared_ptr<audio::river::io::Group> out;
	std::map<std::string, std11::shared_ptr<audio::river::io::Group> >::iterator it = m_listGroup.find(_name);
	if (it == m_listGroup.end()) {
		RIVER_INFO("Create a new group: " << _name << " (START)");
		out = std11::make_shared<audio::river::io::Group>();
		if (out != nullptr) {
			out->createFrom(m_config, _name);
			std::pair<std::string, std11::shared_ptr<audio::river::io::Group> > plop(std::string(_name), out);
			m_listGroup.insert(plop);
			RIVER_INFO("Create a new group: " << _name << " ( END )");
		} else {
			RIVER_ERROR("Can not create new group: " << _name << " ( END )");
		}
	} else {
		out = it->second;
	}
	
	return out;
}