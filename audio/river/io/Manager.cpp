/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <audio/river/io/Manager.hpp>
#include <audio/river/debug.hpp>
#include <audio/river/river.hpp>
#include <audio/river/io/Node.hpp>
#include <audio/river/io/NodeAEC.hpp>
#include <audio/river/io/NodeMuxer.hpp>
#include <audio/river/io/NodeOrchestra.hpp>
#include <audio/river/io/NodePortAudio.hpp>
#include <etk/os/FSNode.hpp>
#include <ememory/memory.hpp>
#include <etk/types.hpp>

#ifdef AUDIO_RIVER_BUILD_PORTAUDIO
	extern "C" {
		#include <portaudio/portaudio.h>
	}
#endif

static etk::String basicAutoConfig =
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


static etk::String pathToTheRiverConfigInHome("HOME:.local/share/audio-river/config.json");

audio::river::io::Manager::Manager() {
	#ifdef AUDIO_RIVER_BUILD_PORTAUDIO
	PaError err = Pa_Initialize();
	if(err != paNoError) {
		RIVER_WARNING("Can not initialize portaudio : " << Pa_GetErrorText(err));
	}
	#endif
}

void audio::river::io::Manager::init(const etk::String& _filename) {
	RIVER_ERROR("kjqsdhfkjqshdfkjqhsdskjdfhfkqjshqhskdjfhqsdfqsdqsdfqsdqsdfqsdfqsdfqsdfqsdfqsd");
	ethread::RecursiveLock lock(m_mutex);
	if (_filename == "") {
		if (m_config.load(pathToTheRiverConfigInHome) == false) {
			RIVER_INFO("Load default config");
			m_config.parse(basicAutoConfig);
		} else {
			RIVER_INFO("Load default user configuration: " << pathToTheRiverConfigInHome);
		}
	} else if (m_config.load(_filename) == false) {
		RIVER_ERROR("you must set a basic configuration file for harware configuration: '" << _filename << "'");
	}
}

void audio::river::io::Manager::initString(const etk::String& _data) {
	ethread::RecursiveLock lock(m_mutex);
	m_config.parse(_data);
}

void audio::river::io::Manager::unInit() {
	ethread::RecursiveLock lock(m_mutex);
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
	static ememory::SharedPtr<audio::river::io::Manager> manager(ETK_NEW(Manager));
	return manager;
}


etk::Vector<etk::String> audio::river::io::Manager::getListStreamInput() {
	ethread::RecursiveLock lock(m_mutex);
	etk::Vector<etk::String> output;
	etk::Vector<etk::String> keys = m_config.getKeys();
	for (auto &it : keys) {
		const ejson::Object tmppp = m_config[it].toObject();
		if (tmppp.exist() == true) {
			etk::String type = tmppp["io"].toString().get("error");
			if (    type == "input"
			     || type == "PAinput") {
				output.pushBack(it);
			}
		}
	}
	return output;
}

etk::Vector<etk::String> audio::river::io::Manager::getListStreamOutput() {
	ethread::RecursiveLock lock(m_mutex);
	etk::Vector<etk::String> output;
	etk::Vector<etk::String> keys = m_config.getKeys();
	for (auto &it : keys) {
		const ejson::Object tmppp = m_config[it].toObject();
		if (tmppp.exist() == true) {
			etk::String type = tmppp["io"].toString().get("error");
			if (    type == "output"
			     || type == "PAoutput") {
				output.pushBack(it);
			}
		}
	}
	return output;
}

etk::Vector<etk::String> audio::river::io::Manager::getListStreamVirtual() {
	ethread::RecursiveLock lock(m_mutex);
	etk::Vector<etk::String> output;
	etk::Vector<etk::String> keys = m_config.getKeys();
	for (auto &it : keys) {
		const ejson::Object tmppp = m_config[it].toObject();
		if (tmppp.exist() == true) {
			etk::String type = tmppp["io"].toString().get("error");
			if (    type != "input"
			     && type != "PAinput"
			     && type != "output"
			     && type != "PAoutput"
			     && type != "error") {
				output.pushBack(it);
			}
		}
	}
	return output;
}

etk::Vector<etk::String> audio::river::io::Manager::getListStream() {
	ethread::RecursiveLock lock(m_mutex);
	etk::Vector<etk::String> output;
	etk::Vector<etk::String> keys = m_config.getKeys();
	for (auto &it : keys) {
		const ejson::Object tmppp = m_config[it].toObject();
		if (tmppp.exist() == true) {
			etk::String type = tmppp["io"].toString().get("error");
			if (type != "error") {
				output.pushBack(it);
			}
		}
	}
	return output;
}

ememory::SharedPtr<audio::river::io::Node> audio::river::io::Manager::getNode(const etk::String& _name) {
	ethread::RecursiveLock lock(m_mutex);
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
		for (etk::Map<etk::String, ememory::SharedPtr<audio::river::io::Group> >::Iterator it(m_listGroup.begin());
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
		etk::String groupName = tmpObject["group"].toString().get();
		// get type : io
		etk::String ioType = tmpObject["io"].toString().get("error");
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
					m_list.pushBack(tmp);
					return tmp;
				#else
					RIVER_WARNING("not present interface");
				#endif
			}
			if (    ioType == "PAinput"
			     || ioType == "PAoutput") {
				#ifdef AUDIO_RIVER_BUILD_PORTAUDIO
					ememory::SharedPtr<audio::river::io::Node> tmp = audio::river::io::NodePortAudio::create(_name, tmpObject);
					m_list.pushBack(tmp);
					return tmp;
				#else
					RIVER_WARNING("not present interface");
				#endif
			}
			if (ioType == "aec") {
				ememory::SharedPtr<audio::river::io::Node> tmp = audio::river::io::NodeAEC::create(_name, tmpObject);
				m_list.pushBack(tmp);
				return tmp;
			}
			if (ioType == "muxer") {
				ememory::SharedPtr<audio::river::io::Node> tmp = audio::river::io::NodeMuxer::create(_name, tmpObject);
				m_list.pushBack(tmp);
				return tmp;
			}
		}
	}
	RIVER_ERROR("Can not create the interface : '" << _name << "' the node is not DEFINED in the configuration file availlable : " << m_config.getKeys());
	return ememory::SharedPtr<audio::river::io::Node>();
}

ememory::SharedPtr<audio::drain::VolumeElement> audio::river::io::Manager::getVolumeGroup(const etk::String& _name) {
	ethread::RecursiveLock lock(m_mutex);
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
	m_volumeGroup.pushBack(tmpVolume);
	return tmpVolume;
}

bool audio::river::io::Manager::setVolume(const etk::String& _volumeName, float _valuedB) {
	ethread::RecursiveLock lock(m_mutex);
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

float audio::river::io::Manager::getVolume(const etk::String& _volumeName) {
	ethread::RecursiveLock lock(m_mutex);
	ememory::SharedPtr<audio::drain::VolumeElement> volume = getVolumeGroup(_volumeName);
	if (volume == nullptr) {
		RIVER_ERROR("Can not get volume ... : '" << _volumeName << "'");
		return 0.0f;
	}
	return volume->getVolume();
}

etk::Pair<float,float> audio::river::io::Manager::getVolumeRange(const etk::String& _volumeName) const {
	return etk::makePair<float,float>(-300, 300);
}

void audio::river::io::Manager::setMute(const etk::String& _volumeName, bool _mute) {
	ethread::RecursiveLock lock(m_mutex);
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

bool audio::river::io::Manager::getMute(const etk::String& _volumeName) {
	ethread::RecursiveLock lock(m_mutex);
	ememory::SharedPtr<audio::drain::VolumeElement> volume = getVolumeGroup(_volumeName);
	if (volume == nullptr) {
		RIVER_ERROR("Can not get volume ... : '" << _volumeName << "'");
		return false;
	}
	return volume->getMute();
}

void audio::river::io::Manager::generateDot(const etk::String& _filename) {
	ethread::RecursiveLock lock(m_mutex);
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
		for (etk::Map<etk::String, ememory::SharedPtr<audio::river::io::Group> >::Iterator it(m_listGroup.begin());
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
		for (etk::Map<etk::String, ememory::SharedPtr<audio::river::io::Group> >::Iterator it(m_listGroup.begin());
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

ememory::SharedPtr<audio::river::io::Group> audio::river::io::Manager::getGroup(const etk::String& _name) {
	ethread::RecursiveLock lock(m_mutex);
	ememory::SharedPtr<audio::river::io::Group> out;
	etk::Map<etk::String, ememory::SharedPtr<audio::river::io::Group> >::Iterator it = m_listGroup.find(_name);
	if (it == m_listGroup.end()) {
		RIVER_INFO("Create a new group: " << _name << " (START)");
		out = ememory::makeShared<audio::river::io::Group>();
		if (out != nullptr) {
			out->createFrom(m_config, _name);
			m_listGroup.add(_name, out);
			RIVER_INFO("Create a new group: " << _name << " ( END )");
		} else {
			RIVER_ERROR("Can not create new group: " << _name << " ( END )");
		}
	} else {
		out = it->second;
	}
	
	return out;
}
