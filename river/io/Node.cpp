/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "Node.h"
#include <river/debug.h>

#include <memory>

#undef __class__
#define __class__ "io::Node"


river::io::Node::Node(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config) :
  m_config(_config),
  m_name(_name),
  m_isInput(false) {
	RIVER_INFO("-----------------------------------------------------------------");
	RIVER_INFO("--                       CREATE NODE                           --");
	RIVER_INFO("-----------------------------------------------------------------");
	drain::IOFormatInterface interfaceFormat;
	drain::IOFormatInterface hardwareFormat;
	/**
		io:"input", # input, output or aec
		frequency:48000, # frequency to open device
		channel-map:[ # mapping of the harware device (to change map if needed)
			"front-left", "front-right",
			"read-left", "rear-right",
		],
		# format to open device (int8, int16, int16-on-ont32, int24, int32, float)
		type:"int16",
		# muxer/demuxer format type (int8-on-int16, int16-on-int32, int24-on-int32, int32-on-int64, float)
		mux-demux-type:"int16_on_int32", 
	*/
	std::string interfaceType = m_config->getStringValue("io");
	if (    interfaceType == "input"
	     || interfaceType == "aec") {
		m_isInput = true;
	} else {
		m_isInput = false;
	}
	
	int32_t frequency = m_config->getNumberValue("frequency", 1);
	// Get audio format type:
	std::string type = m_config->getStringValue("type", "int16");
	enum audio::format formatType = audio::getFormatFromString(type);
	// Get volume stage :
	std::string volumeName = m_config->getStringValue("volume-name", "");
	if (volumeName != "") {
		RIVER_INFO("add node volume stage : '" << volumeName << "'");
		// use global manager for volume ...
		m_volume = river::io::Manager::getInstance()->getVolumeGroup(volumeName);
	}
	// Get map type :
	std::vector<audio::channel> map;
	const std::shared_ptr<const ejson::Array> listChannelMap = m_config->getArray("channel-map");
	if (    listChannelMap == nullptr
	     || listChannelMap->size() == 0) {
		// set default channel property:
		map.push_back(audio::channel_frontLeft);
		map.push_back(audio::channel_frontRight);
	} else {
		for (size_t iii=0; iii<listChannelMap->size(); ++iii) {
			std::string value = listChannelMap->getStringValue(iii);
			map.push_back(audio::getChannelFromString(value));
		}
	}
	hardwareFormat.set(map, formatType, frequency);
	
	
	std::string muxerDemuxerConfig = m_config->getStringValue("mux-demux-type", "int16-on-int32");
	enum audio::format muxerFormatType = audio::getFormatFromString(muxerDemuxerConfig);
	if (m_isInput == true) {
		if (muxerFormatType != audio::format_int16) {
			RIVER_CRITICAL("not supported demuxer type ... " << muxerFormatType << " for INPUT set in file:" << muxerDemuxerConfig);
		}
	} else {
		if (muxerFormatType != audio::format_int16_on_int32) {
			RIVER_CRITICAL("not supported demuxer type ... " << muxerFormatType << " for OUTPUT set in file:" << muxerDemuxerConfig);
		}
	}
	// no map change and no frequency change ...
	interfaceFormat.set(map, muxerFormatType, frequency);
	// configure process interface
	if (m_isInput == true) {
		m_process.setInputConfig(hardwareFormat);
		m_process.setOutputConfig(interfaceFormat);
	} else {
		m_process.setOutputConfig(hardwareFormat);
		m_process.setInputConfig(interfaceFormat);
	}
	//m_process.updateInterAlgo();
}

river::io::Node::~Node() {
	RIVER_INFO("-----------------------------------------------------------------");
	RIVER_INFO("--                      DESTROY NODE                           --");
	RIVER_INFO("-----------------------------------------------------------------");
};

void river::io::Node::registerAsRemote(const std::shared_ptr<river::Interface>& _interface) {
	auto it = m_listAvaillable.begin();
	while (it != m_listAvaillable.end()) {
		if (it->expired() == true) {
			it = m_listAvaillable.erase(it);
		}
		++it;
	}
	m_listAvaillable.push_back(_interface);
}

void river::io::Node::interfaceAdd(const std::shared_ptr<river::Interface>& _interface) {
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (_interface == m_list[iii]) {
				return;
			}
		}
		RIVER_INFO("ADD interface for stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
		m_list.push_back(_interface);
	}
	if (m_list.size() == 1) {
		start();
	}
}

void river::io::Node::interfaceRemove(const std::shared_ptr<river::Interface>& _interface) {
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (_interface == m_list[iii]) {
				m_list.erase(m_list.begin()+iii);
				RIVER_INFO("RM interface for stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
				break;
			}
		}
	}
	if (m_list.size() == 0) {
		stop();
	}
	return;
}


void river::io::Node::volumeChange() {
	for (auto &it : m_listAvaillable) {
		std::shared_ptr<river::Interface> node = it.lock();
		if (node != nullptr) {
			node->systemVolumeChange();
		}
	}
}

int32_t river::io::Node::newInput(const void* _inputBuffer,
                                  uint32_t _nbChunk,
                                  const std::chrono::system_clock::time_point& _time) {
	if (_inputBuffer == nullptr) {
		return -1;
	}
	const int16_t* inputBuffer = static_cast<const int16_t *>(_inputBuffer);
	for (auto &it : m_list) {
		if (it == nullptr) {
			continue;
		}
		if (it->getMode() != river::modeInterface_input) {
			continue;
		}
		RIVER_VERBOSE("    IO name="<< it->getName());
		it->systemNewInputData(_time, inputBuffer, _nbChunk);
	}
	RIVER_VERBOSE("data Input size request :" << _nbChunk << " [ END ]");
	return 0;
}

int32_t river::io::Node::newOutput(void* _outputBuffer,
                                   uint32_t _nbChunk,
                                   const std::chrono::system_clock::time_point& _time) {
	if (_outputBuffer == nullptr) {
		return -1;
	}
	std::vector<int32_t> output;
	RIVER_VERBOSE("resize=" << _nbChunk*m_process.getInputConfig().getMap().size());
	output.resize(_nbChunk*m_process.getInputConfig().getMap().size(), 0);
	const int32_t* outputTmp = nullptr;
	std::vector<uint8_t> outputTmp2;
	RIVER_VERBOSE("resize=" << sizeof(int32_t)*m_process.getInputConfig().getMap().size()*_nbChunk);
	outputTmp2.resize(sizeof(int32_t)*m_process.getInputConfig().getMap().size()*_nbChunk, 0);
	for (auto &it : m_list) {
		if (it == nullptr) {
			continue;
		}
		if (it->getMode() != river::modeInterface_output) {
			continue;
		}
		RIVER_VERBOSE("    IO name="<< it->getName());
		// clear datas ...
		memset(&outputTmp2[0], 0, sizeof(int32_t)*m_process.getInputConfig().getMap().size()*_nbChunk);
		RIVER_VERBOSE("        request Data="<< _nbChunk);
		it->systemNeedOutputData(_time, &outputTmp2[0], _nbChunk, sizeof(int32_t)*m_process.getInputConfig().getMap().size());
		RIVER_VERBOSE("        Mix it ...");
		outputTmp = reinterpret_cast<const int32_t*>(&outputTmp2[0]);
		// Add data to the output tmp buffer :
		for (size_t kkk=0; kkk<output.size(); ++kkk) {
			output[kkk] += outputTmp[kkk];
		}
	}
	RIVER_VERBOSE("    End stack process data ...");
	m_process.processIn(&outputTmp2[0], _nbChunk, _outputBuffer, _nbChunk);
	RIVER_VERBOSE("    Feedback :");
	for (auto &it : m_list) {
		if (it == nullptr) {
			continue;
		}
		if (it->getMode() != river::modeInterface_feedback) {
			continue;
		}
		RIVER_VERBOSE("    IO name="<< it->getName() << " (feedback)");
		it->systemNewInputData(_time, _outputBuffer, _nbChunk);
	}
	RIVER_VERBOSE("data Output size request :" << _nbChunk << " [ END ]");
	return 0;
}

void river::io::Node::generateDot(etk::FSNode& _node) {
	_node << "subgraph cluster_0 {\n";
	// configure display:
	_node << "	node [shape=record, fontname=Helvetica, fontsize=10, color=lightsteelblue1, style=filled];\n";
	//_node << "	node [shape=diamond, fontname=Helvetica, fontsize=10, color=orangered, style=filled];\n"
	//_node << "	node [shape=ellipse, fontname=Helvetica, fontsize=8, color=aquamarine2, style=filled];\n";
	// add elements
	int32_t idNode = 0;
	_node << "		NODE_" << idNode << " [ label=\"name=" << m_name << "\" ];\n";
	// add IO
	_node << "	node [shape=ellipse, fontname=Helvetica, fontsize=8, color=aquamarine2, style=filled];\n";
	int32_t id = 0;
	for (auto &it : m_list) {
		if (it != nullptr) {
			_node << "		interface_" << id << " [ label=\"name=" << it->getName() << "\" ];\n";
			_node << "		NODE_" << idNode << " -> interface_" << id << " [ arrowhead=\"open\"];\n";
		}
	}
	_node << "}\n";
}

