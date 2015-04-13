/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "Node.h"
#include <audio/river/debug.h>

#undef __class__
#define __class__ "io::Node"


audio::river::io::Node::Node(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config) :
  m_config(_config),
  m_name(_name),
  m_isInput(false) {
	static uint32_t uid=0;
	m_uid = uid++;
	RIVER_INFO("-----------------------------------------------------------------");
	RIVER_INFO("--                       CREATE NODE                           --");
	RIVER_INFO("-----------------------------------------------------------------");
	audio::drain::IOFormatInterface interfaceFormat;
	audio::drain::IOFormatInterface hardwareFormat;
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
	RIVER_INFO("interfaceType=" << interfaceType);
	if (    interfaceType == "input"
	     || interfaceType == "PAinput"
	     || interfaceType == "aec"
	     || interfaceType == "muxer") {
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
		m_volume = audio::river::io::Manager::getInstance()->getVolumeGroup(volumeName);
	}
	// Get map type :
	std::vector<audio::channel> map;
	const std11::shared_ptr<const ejson::Array> listChannelMap = m_config->getArray("channel-map");
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
	
	
	std::string muxerDemuxerConfig;
	if (m_isInput == true) {
		muxerDemuxerConfig = m_config->getStringValue("mux-demux-type", "int16");
	} else {
		muxerDemuxerConfig = m_config->getStringValue("mux-demux-type", "int16-on-int32");
	}
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

audio::river::io::Node::~Node() {
	RIVER_INFO("-----------------------------------------------------------------");
	RIVER_INFO("--                      DESTROY NODE                           --");
	RIVER_INFO("-----------------------------------------------------------------");
};

size_t audio::river::io::Node::getNumberOfInterface(enum audio::river::modeInterface _interfaceType) {
	size_t out = 0;
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		if (m_list[iii] == nullptr) {
			continue;
		}
		if (m_list[iii]->getMode() == _interfaceType) {
			out++;
		}
	}
	return out;
}
size_t audio::river::io::Node::getNumberOfInterfaceAvaillable(enum audio::river::modeInterface _interfaceType) {
	size_t out = 0;
	for (size_t iii=0; iii<m_listAvaillable.size(); ++iii) {
		std11::shared_ptr<audio::river::Interface> element = m_listAvaillable[iii].lock();
		if (element == nullptr) {
			continue;
		}
		if (element->getMode() == _interfaceType) {
			out++;
		}
	}
	return out;
}

void audio::river::io::Node::registerAsRemote(const std11::shared_ptr<audio::river::Interface>& _interface) {
	std::vector<std11::weak_ptr<audio::river::Interface> >::iterator it = m_listAvaillable.begin();
	while (it != m_listAvaillable.end()) {
		if (it->expired() == true) {
			it = m_listAvaillable.erase(it);
			continue;
		}
		++it;
	}
	m_listAvaillable.push_back(_interface);
}

void audio::river::io::Node::interfaceAdd(const std11::shared_ptr<audio::river::Interface>& _interface) {
	{
		std11::unique_lock<std11::mutex> lock(m_mutex);
		for (size_t iii=0; iii<m_list.size(); ++iii) {
			if (_interface == m_list[iii]) {
				return;
			}
		}
		RIVER_INFO("ADD interface for stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
		m_list.push_back(_interface);
	}
	if (m_list.size() == 1) {
		startInGroup();
	}
}

void audio::river::io::Node::interfaceRemove(const std11::shared_ptr<audio::river::Interface>& _interface) {
	{
		std11::unique_lock<std11::mutex> lock(m_mutex);
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (_interface == m_list[iii]) {
				m_list.erase(m_list.begin()+iii);
				RIVER_INFO("RM interface for stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
				break;
			}
		}
	}
	if (m_list.size() == 0) {
		stopInGroup();
	}
	return;
}


void audio::river::io::Node::volumeChange() {
	for (size_t iii=0; iii< m_listAvaillable.size(); ++iii) {
		std11::shared_ptr<audio::river::Interface> node = m_listAvaillable[iii].lock();
		if (node != nullptr) {
			node->systemVolumeChange();
		}
	}
}

void audio::river::io::Node::newInput(const void* _inputBuffer,
                               uint32_t _nbChunk,
                               const audio::Time& _time) {
	if (_inputBuffer == nullptr) {
		return;
	}
	const int16_t* inputBuffer = static_cast<const int16_t *>(_inputBuffer);
	for (size_t iii=0; iii< m_list.size(); ++iii) {
		if (m_list[iii] == nullptr) {
			continue;
		}
		if (m_list[iii]->getMode() != audio::river::modeInterface_input) {
			continue;
		}
		RIVER_VERBOSE("    IO name="<< m_list[iii]->getName());
		m_list[iii]->systemNewInputData(_time, inputBuffer, _nbChunk);
	}
	RIVER_VERBOSE("data Input size request :" << _nbChunk << " [ END ]");
	return;
}

void audio::river::io::Node::newOutput(void* _outputBuffer,
                                uint32_t _nbChunk,
                                const audio::Time& _time) {
	if (_outputBuffer == nullptr) {
		return;
	}
	std::vector<int32_t> output;
	RIVER_VERBOSE("resize=" << _nbChunk*m_process.getInputConfig().getMap().size());
	output.resize(_nbChunk*m_process.getInputConfig().getMap().size(), 0);
	// TODO : set here the mixer selection ...
	if (true) {
		const int32_t* outputTmp = nullptr;
		std::vector<uint8_t> outputTmp2;
		RIVER_VERBOSE("resize=" << sizeof(int32_t)*m_process.getInputConfig().getMap().size()*_nbChunk);
		outputTmp2.resize(sizeof(int32_t)*m_process.getInputConfig().getMap().size()*_nbChunk, 0);
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (m_list[iii] == nullptr) {
				continue;
			}
			if (m_list[iii]->getMode() != audio::river::modeInterface_output) {
				continue;
			}
			RIVER_VERBOSE("    IO name="<< m_list[iii]->getName() << " " << iii);
			// clear datas ...
			memset(&outputTmp2[0], 0, sizeof(int32_t)*m_process.getInputConfig().getMap().size()*_nbChunk);
			RIVER_VERBOSE("        request Data="<< _nbChunk << " time=" << _time);
			m_list[iii]->systemNeedOutputData(_time, &outputTmp2[0], _nbChunk, sizeof(int32_t)*m_process.getInputConfig().getMap().size());
			outputTmp = reinterpret_cast<const int32_t*>(&outputTmp2[0]);
			RIVER_VERBOSE("        Mix it ...");
			// Add data to the output tmp buffer :
			for (size_t kkk=0; kkk<output.size(); ++kkk) {
				output[kkk] += outputTmp[kkk];
			}
		}
	}
	RIVER_VERBOSE("    End stack process data ...");
	m_process.processIn(&output[0], _nbChunk, _outputBuffer, _nbChunk);
	RIVER_VERBOSE("    Feedback :");
	for (size_t iii=0; iii< m_list.size(); ++iii) {
		if (m_list[iii] == nullptr) {
			continue;
		}
		if (m_list[iii]->getMode() != audio::river::modeInterface_feedback) {
			continue;
		}
		RIVER_VERBOSE("    IO name="<< m_list[iii]->getName() << " (feedback) time=" << _time);
		m_list[iii]->systemNewInputData(_time, _outputBuffer, _nbChunk);
	}
	RIVER_VERBOSE("data Output size request :" << _nbChunk << " [ END ]");
	return;
}

static void link(etk::FSNode& _node, const std::string& _first, const std::string& _op, const std::string& _second) {
	if (_op == "->") {
		_node << "			" << _first << " -> " << _second << ";\n";
	} else if (_op == "<-") {
		_node << "			" << _first << " -> " <<_second<< " [color=transparent];\n";
		_node << "			" << _second << " -> " << _first << " [constraint=false];\n";
	}
}


void audio::river::io::Node::generateDot(etk::FSNode& _node) {
	_node << "	subgraph clusterNode_" << m_uid << " {\n";
	_node << "		color=blue;\n";
	_node << "		label=\"[" << m_uid << "] IO::Node : " << m_name << "\";\n";
	if (m_isInput == true) {
		_node << "		node [shape=rarrow];\n";
		_node << "			NODE_" << m_uid << "_HW_interface [ label=\"HW interface\\n interface=ALSA\\n stream=" << m_name << "\\n type=input\" ];\n";
		std::string nameIn;
		std::string nameOut;
		m_process.generateDotProcess(_node, 3, m_uid, nameIn, nameOut, false);
		_node << "		node [shape=square];\n";
		_node << "			NODE_" << m_uid << "_demuxer [ label=\"DEMUXER\\n format=" << etk::to_string(m_process.getOutputConfig().getFormat()) << "\" ];\n";
		// Link all nodes :
		_node << "			NODE_" << m_uid << "_HW_interface -> " << nameIn << " [arrowhead=\"open\"];\n";
		_node << "			" << nameOut << " -> NODE_" << m_uid << "_demuxer [arrowhead=\"open\"];\n";
	} else {
		size_t nbOutput = getNumberOfInterfaceAvaillable(audio::river::modeInterface_output);
		size_t nbfeedback = getNumberOfInterfaceAvaillable(audio::river::modeInterface_feedback);
		_node << "		node [shape=larrow];\n";
		_node << "			NODE_" << m_uid << "_HW_interface [ label=\"HW interface\\n interface=ALSA\\n stream=" << m_name << "\\n type=output\" ];\n";
		std::string nameIn;
		std::string nameOut;
		if (nbOutput>0) {
			m_process.generateDotProcess(_node, 3, m_uid, nameIn, nameOut, true);
		}
		_node << "		node [shape=square];\n";
		if (nbOutput>0) {
			_node << "			NODE_" << m_uid << "_muxer [ label=\"MUXER\\n format=" << etk::to_string(m_process.getInputConfig().getFormat()) << "\" ];\n";
		}
		if (nbfeedback>0) {
			_node << "			NODE_" << m_uid << "_demuxer [ label=\"DEMUXER\\n format=" << etk::to_string(m_process.getOutputConfig().getFormat()) << "\" ];\n";
		}
		// Link all nodes :
		if (nbOutput>0) {
			link(_node, "NODE_" + etk::to_string(m_uid) + "_HW_interface", "<-", nameOut);
			link(_node, nameIn, "<-", "NODE_" + etk::to_string(m_uid) + "_muxer");
		}
		if (nbfeedback>0) {
			_node << "			NODE_" << m_uid << "_HW_interface -> NODE_" << m_uid << "_demuxer [arrowhead=\"open\"];\n";
		}
		if (    nbOutput>0
		     && nbfeedback>0) {
			_node << "			{ rank=same; NODE_" << m_uid << "_demuxer; NODE_" << m_uid << "_muxer }\n";
		}
		
	}
	_node << "	}\n	\n";
	
	for (size_t iii=0; iii< m_listAvaillable.size(); ++iii) {
		if (m_listAvaillable[iii].expired() == true) {
			continue;
		}
		std11::shared_ptr<audio::river::Interface> element = m_listAvaillable[iii].lock();
		if (element == nullptr) {
			continue;
		}
		bool isLink = false;
		for (size_t jjj=0; jjj<m_list.size(); ++jjj) {
			if (element == m_list[jjj]) {
				isLink = true;
			}
		}
		if (element != nullptr) {
			if (element->getMode() == modeInterface_input) {
				element->generateDot(_node, "NODE_" + etk::to_string(m_uid) + "_demuxer", isLink);
			} else if (element->getMode() == modeInterface_output) {
				element->generateDot(_node, "NODE_" + etk::to_string(m_uid) + "_muxer", isLink);
			} else if (element->getMode() == modeInterface_feedback) {
				element->generateDot(_node, "NODE_" + etk::to_string(m_uid) + "_demuxer", isLink);
			} else {
				
			}
		}
	}
}


void audio::river::io::Node::startInGroup() {
	std11::shared_ptr<audio::river::io::Group> group = m_group.lock();
	if (group != nullptr) {
		group->start();
	} else {
		start();
	}
}

void audio::river::io::Node::stopInGroup() {
	std11::shared_ptr<audio::river::io::Group> group = m_group.lock();
	if (group != nullptr) {
		group->stop();
	} else {
		stop();
	}
}