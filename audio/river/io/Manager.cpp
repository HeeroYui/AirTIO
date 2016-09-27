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
#include <ememory/memory.h>
#include <etk/types.h>
#include <utility>

#ifdef AUDIO_RIVER_BUILD_PORTAUDIO
	#include <portaudio/portaudio.h>
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
	"		nb-chunk:1024,\n"
	"		mux-demux-type:'float'\n"
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
	"		volume-name:'MASTER',\n"
	"		mux-demux-type:'float'\n"
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
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	if (_filename == "") {
		RIVER_INFO("Load default config");
		m_config.parse(basicAutoConfig);
	} else if (m_config.load(_filename) == false) {
		RIVER_ERROR("you must set a basic configuration file for harware configuration: '" << _filename << "'");
	}
}

void audio::river::io::Manager::initString(const std::string& _data) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_config.parse(_data);
}

void audio::river::io::Manager::unInit() {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
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

ememory::SharedPtr<audio::river::io::Manager> audio::river::io::Manager::getInstance() {
	if (audio::river::isInit() == false) {
		return ememory::SharedPtr<audio::river::io::Manager>();
	}
	static ememory::SharedPtr<audio::river::io::Manager> manager(new Manager());
	return manager;
}


std::vector<std::string> audio::river::io::Manager::getListStreamInput() {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	std::vector<std::string> output;
	std::vector<std::string> keys = m_config.getKeys();
	for (auto &it : keys) {
		const ejson::Object tmppp = m_config[it].toObject();
		if (tmppp.exist() == true) {
			std::string type = tmppp["io"].toString().get("error");
			if (    type == "input"
			     || type == "PAinput") {
				output.push_back(it);
			}
		}
	}
	return output;
}

std::vector<std::string> audio::river::io::Manager::getListStreamOutput() {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	std::vector<std::string> output;
	std::vector<std::string> keys = m_config.getKeys();
	for (auto &it : keys) {
		const ejson::Object tmppp = m_config[it].toObject();
		if (tmppp.exist() == true) {
			std::string type = tmppp["io"].toString().get("error");
			if (    type == "output"
			     || type == "PAoutput") {
				output.push_back(it);
			}
		}
	}
	return output;
}

std::vector<std::string> audio::river::io::Manager::getListStreamVirtual() {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	std::vector<std::string> output;
	std::vector<std::string> keys = m_config.getKeys();
	for (auto &it : keys) {
		const ejson::Object tmppp = m_config[it].toObject();
		if (tmppp.exist() == true) {
			std::string type = tmppp["io"].toString().get("error");
			if (    type != "input"
			     && type != "PAinput"
			     && type != "output"
			     && type != "PAoutput"
			     && type != "error") {
				output.push_back(it);
			}
		}
	}
	return output;
}

std::vector<std::string> audio::river::io::Manager::getListStream() {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	std::vector<std::string> output;
	std::vector<std::string> keys = m_config.getKeys();
	for (auto &it : keys) {
		const ejson::Object tmppp = m_config[it].toObject();
		if (tmppp.exist() == true) {
			std::string type = tmppp["io"].toString().get("error");
			if (type != "error") {
				output.push_back(it);
			}
		}
	}
	return output;
}

ememory::SharedPtr<audio::river::io::Node> audio::river::io::Manager::getNode(const std::string& _name) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	RIVER_WARNING("Get node : " << _name);
	// search in the standalone list :
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		ememory::SharedPtr<audio::river::io::Node> tmppp = m_list[iii].lock();
		if (    tmppp != nullptr
		     && _name == tmppp->getName()) {
			RIVER_WARNING(" find it ... in standalone");
			return tmppp;
		}
	}
	// search in the group list:
	{
		for (std::map<std::string, ememory::SharedPtr<audio::river::io::Group> >::iterator it(m_listGroup.begin());
		     it != m_listGroup.end();
		     ++it) {
			if (it->second != nullptr) {
				ememory::SharedPtr<audio::river::io::Node> node = it->second->getNode(_name);
				if (node != nullptr) {
					RIVER_WARNING(" find it ... in group: " << it->first);
					return node;
				}
			}
		}
	}
	RIVER_WARNING("Try create a new one : " << _name);
	// check if the node can be open :
	const ejson::Object tmpObject = m_config[_name].toObject();
	if (tmpObject.exist() == true) {
		//Check if it is in a group:
		std::string groupName = tmpObject["group"].toString().get();
		// get type : io
		std::string ioType = tmpObject["io"].toString().get("error");
		if (    groupName != ""
		     && (    ioType == "input"
		          || ioType == "output"
		          || ioType == "PAinput"
		          || ioType == "PAoutput") ) {
			ememory::SharedPtr<audio::river::io::Group> tmpGroup = getGroup(groupName);
			if (tmpGroup == nullptr) {
				RIVER_WARNING("Can not get group ... '" << groupName << "'");
				return ememory::SharedPtr<audio::river::io::Node>();
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
					ememory::SharedPtr<audio::river::io::Node> tmp = audio::river::io::NodeOrchestra::create(_name, tmpObject);
					m_list.push_back(tmp);
					return tmp;
				#else
					RIVER_WARNING("not present interface");
				#endif
			}
			if (    ioType == "PAinput"
			     || ioType == "PAoutput") {
				#ifdef AUDIO_RIVER_BUILD_PORTAUDIO
					ememory::SharedPtr<audio::river::io::Node> tmp = audio::river::io::NodePortAudio::create(_name, tmpObject);
					m_list.push_back(tmp);
					return tmp;
				#else
					RIVER_WARNING("not present interface");
				#endif
			}
			if (ioType == "aec") {
				ememory::SharedPtr<audio::river::io::Node> tmp = audio::river::io::NodeAEC::create(_name, tmpObject);
				m_list.push_back(tmp);
				return tmp;
			}
			if (ioType == "muxer") {
				ememory::SharedPtr<audio::river::io::Node> tmp = audio::river::io::NodeMuxer::create(_name, tmpObject);
				m_list.push_back(tmp);
				return tmp;
			}
		}
	}
	RIVER_ERROR("Can not create the interface : '" << _name << "' the node is not DEFINED in the configuration file availlable : " << m_config.getKeys());
	return ememory::SharedPtr<audio::river::io::Node>();
}

ememory::SharedPtr<audio::drain::VolumeElement> audio::river::io::Manager::getVolumeGroup(const std::string& _name) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	if (_name == "") {
		RIVER_ERROR("Try to create an audio group with no name ...");
		return ememory::SharedPtr<audio::drain::VolumeElement>();
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
	ememory::SharedPtr<audio::drain::VolumeElement> tmpVolume = ememory::makeShared<audio::drain::VolumeElement>(_name);
	m_volumeGroup.push_back(tmpVolume);
	return tmpVolume;
}

bool audio::river::io::Manager::setVolume(const std::string& _volumeName, float _valuedB) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	ememory::SharedPtr<audio::drain::VolumeElement> volume = getVolumeGroup(_volumeName);
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
		ememory::SharedPtr<audio::river::io::Node> val = m_list[iii].lock();
		if (val != nullptr) {
			val->volumeChange();
		}
	}
	return true;
}

float audio::river::io::Manager::getVolume(const std::string& _volumeName) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	ememory::SharedPtr<audio::drain::VolumeElement> volume = getVolumeGroup(_volumeName);
	if (volume == nullptr) {
		RIVER_ERROR("Can not get volume ... : '" << _volumeName << "'");
		return 0.0f;
	}
	return volume->getVolume();
}

std::pair<float,float> audio::river::io::Manager::getVolumeRange(const std::string& _volumeName) const {
	return std::make_pair<float,float>(-300, 300);
}

void audio::river::io::Manager::setMute(const std::string& _volumeName, bool _mute) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	ememory::SharedPtr<audio::drain::VolumeElement> volume = getVolumeGroup(_volumeName);
	if (volume == nullptr) {
		RIVER_ERROR("Can not set volume ... : '" << _volumeName << "'");
		return;
	}
	volume->setMute(_mute);
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		ememory::SharedPtr<audio::river::io::Node> val = m_list[iii].lock();
		if (val != nullptr) {
			val->volumeChange();
		}
	}
}

bool audio::river::io::Manager::getMute(const std::string& _volumeName) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	ememory::SharedPtr<audio::drain::VolumeElement> volume = getVolumeGroup(_volumeName);
	if (volume == nullptr) {
		RIVER_ERROR("Can not get volume ... : '" << _volumeName << "'");
		return false;
	}
	return volume->getMute();
}

void audio::river::io::Manager::generateDot(const std::string& _filename) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
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
			ememory::SharedPtr<audio::river::io::Node> val = m_list[iii].lock();
			if (val != nullptr) {
				if (val->isHarwareNode() == true) {
					val->generateDot(node);
				}
			}
		}
		for (std::map<std::string, ememory::SharedPtr<audio::river::io::Group> >::iterator it(m_listGroup.begin());
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
			ememory::SharedPtr<audio::river::io::Node> val = m_list[iii].lock();
			if (val != nullptr) {
				if (val->isHarwareNode() == false) {
					val->generateDot(node);
				}
			}
		}
		for (std::map<std::string, ememory::SharedPtr<audio::river::io::Group> >::iterator it(m_listGroup.begin());
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

ememory::SharedPtr<audio::river::io::Group> audio::river::io::Manager::getGroup(const std::string& _name) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	ememory::SharedPtr<audio::river::io::Group> out;
	std::map<std::string, ememory::SharedPtr<audio::river::io::Group> >::iterator it = m_listGroup.find(_name);
	if (it == m_listGroup.end()) {
		RIVER_INFO("Create a new group: " << _name << " (START)");
		out = ememory::makeShared<audio::river::io::Group>();
		if (out != nullptr) {
			out->createFrom(m_config, _name);
			std::pair<std::string, ememory::SharedPtr<audio::river::io::Group> > plop(std::string(_name), out);
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
