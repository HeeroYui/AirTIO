/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include "Node.hpp"
#include <audio/river/debug.hpp>

audio::river::io::Node::Node(const etk::String& _name, const ejson::Object& _config) :
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
	etk::String interfaceType = m_config["io"].toString().get();
	RIVER_INFO("interfaceType=" << interfaceType);
	if (    interfaceType == "input"
	     || interfaceType == "PAinput"
	     || interfaceType == "aec"
	     || interfaceType == "muxer") {
		m_isInput = true;
	} else {
		m_isInput = false;
	}
	
	int32_t frequency = m_config["frequency"].toNumber().get(1);
	// Get audio format type:
	etk::String type = m_config["type"].toString().get("int16");
	enum audio::format formatType = audio::getFormatFromString(type);
	// Get volume stage :
	etk::String volumeName = m_config["volume-name"].toString().get();
	if (volumeName != "") {
		RIVER_INFO("add node volume stage : '" << volumeName << "'");
		// use global manager for volume ...
		m_volume = audio::river::io::Manager::getInstance()->getVolumeGroup(volumeName);
	}
	// Get map type :
	etk::Vector<audio::channel> map;
	const ejson::Array listChannelMap = m_config["channel-map"].toArray();
	if (    listChannelMap.exist() == false
	     || listChannelMap.size() == 0) {
		// set default channel property:
		map.pushBack(audio::channel_frontLeft);
		map.pushBack(audio::channel_frontRight);
	} else {
		for (auto it : listChannelMap) {
			etk::String value = it.toString().get();
			map.pushBack(audio::getChannelFromString(value));
		}
	}
	hardwareFormat.set(map, formatType, frequency);
	
	
	etk::String muxerDemuxerConfig;
	if (m_isInput == true) {
		muxerDemuxerConfig = m_config["mux-demux-type"].toString().get("int16");
	} else {
		muxerDemuxerConfig = m_config["mux-demux-type"].toString().get("int16-on-int32");
	}
	enum audio::format muxerFormatType = audio::getFormatFromString(muxerDemuxerConfig);
	if (m_isInput == true) {
		// Support all ...
	} else {
		if (    muxerFormatType != audio::format_int8_on_int16
		     && muxerFormatType != audio::format_int16_on_int32
		     && muxerFormatType != audio::format_int24_on_int32
		     && muxerFormatType != audio::format_int32_on_int64
		     && muxerFormatType != audio::format_float
		     && muxerFormatType != audio::format_double) {
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
		if (m_list[iii] == null) {
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
		auto element = m_listAvaillable[iii].lock();
		if (element == null) {
			continue;
		}
		if (element->getMode() == _interfaceType) {
			out++;
		}
	}
	return out;
}

void audio::river::io::Node::registerAsRemote(const ememory::SharedPtr<audio::river::Interface>& _interface) {
	auto it = m_listAvaillable.begin();
	while (it != m_listAvaillable.end()) {
		if (it->expired() == true) {
			it = m_listAvaillable.erase(it);
			continue;
		}
		++it;
	}
	m_listAvaillable.pushBack(_interface);
}

void audio::river::io::Node::interfaceAdd(const ememory::SharedPtr<audio::river::Interface>& _interface) {
	{
		ethread::UniqueLock lock(m_mutex);
		for (size_t iii=0; iii<m_list.size(); ++iii) {
			if (_interface == m_list[iii]) {
				return;
			}
		}
		RIVER_INFO("ADD interface for stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
		m_list.pushBack(_interface);
	}
	if (m_list.size() == 1) {
		startInGroup();
	}
}

void audio::river::io::Node::interfaceRemove(const ememory::SharedPtr<audio::river::Interface>& _interface) {
	{
		ethread::UniqueLock lock(m_mutex);
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
}


void audio::river::io::Node::volumeChange() {
	for (size_t iii=0; iii< m_listAvaillable.size(); ++iii) {
		auto node = m_listAvaillable[iii].lock();
		if (node != null) {
			node->systemVolumeChange();
		}
	}
}

void audio::river::io::Node::newInput(const void* _inputBuffer,
                                      uint32_t _nbChunk,
                                      const audio::Time& _time) {
	if (_inputBuffer == null) {
		return;
	}
	for (size_t iii=0; iii< m_list.size(); ++iii) {
		if (m_list[iii] == null) {
			continue;
		}
		if (m_list[iii]->getMode() != audio::river::modeInterface_input) {
			continue;
		}
		RIVER_VERBOSE("    IO name="<< m_list[iii]->getName());
		m_list[iii]->systemNewInputData(_time, _inputBuffer, _nbChunk);
	}
	RIVER_VERBOSE("data Input size request :" << _nbChunk << " [ END ]");
	return;
}

void audio::river::io::Node::newOutput(void* _outputBuffer,
                                       uint32_t _nbChunk,
                                       const audio::Time& _time) {
	if (_outputBuffer == null) {
		return;
	}
	enum audio::format muxerFormatType = m_process.getInputConfig().getFormat();
	etk::Vector<uint8_t> outputTmp2;
	uint32_t nbByteTmpBuffer = audio::getFormatBytes(muxerFormatType)*m_process.getInputConfig().getMap().size()*_nbChunk;
	RIVER_VERBOSE("resize=" << nbByteTmpBuffer);
	outputTmp2.resize(nbByteTmpBuffer);
	
	if (muxerFormatType == audio::format_int8_on_int16) {
		//////////////////////////////////////////////////////////////////////////////////////////////////
		//            process 16 bits
		//////////////////////////////////////////////////////////////////////////////////////////////////
		// $$$$ change the int16
		etk::Vector<int16_t> output;
		RIVER_VERBOSE("resize=" << _nbChunk*m_process.getInputConfig().getMap().size());
		output.resize(_nbChunk*m_process.getInputConfig().getMap().size(), 0);
		// $$$$ change the int16
		const int16_t* outputTmp = null;
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (m_list[iii] == null) {
				continue;
			}
			if (m_list[iii]->getMode() != audio::river::modeInterface_output) {
				continue;
			}
			RIVER_VERBOSE("    IO name="<< m_list[iii]->getName() << " " << iii);
			// clear datas ...
			memset(&outputTmp2[0], 0, nbByteTmpBuffer);
			RIVER_VERBOSE("        request Data="<< _nbChunk << " time=" << _time);
			m_list[iii]->systemNeedOutputData(_time, &outputTmp2[0], _nbChunk, audio::getFormatBytes(muxerFormatType)*m_process.getInputConfig().getMap().size());
			// $$$$ change the int16
			outputTmp = reinterpret_cast<const int16_t*>(&outputTmp2[0]);
			RIVER_VERBOSE("        Mix it ...");
			// Add data to the output tmp buffer:
			for (size_t kkk=0; kkk<output.size(); ++kkk) {
				output[kkk] += outputTmp[kkk];
			}
			// TODO : if a signal is upper than 256* the maximum of 1 it can create a real problem ...
		}
		RIVER_VERBOSE("    End stack process data ...");
		m_process.processIn(&output[0], _nbChunk, _outputBuffer, _nbChunk);
	} else if (    muxerFormatType == audio::format_int16_on_int32
	            || muxerFormatType == audio::format_int24_on_int32) {
		//////////////////////////////////////////////////////////////////////////////////////////////////
		//            process 32 bits
		//////////////////////////////////////////////////////////////////////////////////////////////////
		etk::Vector<int32_t> output;
		RIVER_VERBOSE("resize=" << _nbChunk*m_process.getInputConfig().getMap().size());
		output.resize(_nbChunk*m_process.getInputConfig().getMap().size(), 0);
		const int32_t* outputTmp = null;
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (m_list[iii] == null) {
				continue;
			}
			if (m_list[iii]->getMode() != audio::river::modeInterface_output) {
				continue;
			}
			RIVER_VERBOSE("    IO name="<< m_list[iii]->getName() << " " << iii);
			// clear datas ...
			memset(&outputTmp2[0], 0, nbByteTmpBuffer);
			RIVER_VERBOSE("        request Data="<< _nbChunk << " time=" << _time);
			m_list[iii]->systemNeedOutputData(_time, &outputTmp2[0], _nbChunk, audio::getFormatBytes(muxerFormatType)*m_process.getInputConfig().getMap().size());
			outputTmp = reinterpret_cast<const int32_t*>(&outputTmp2[0]);
			RIVER_VERBOSE("        Mix it ...");
			// Add data to the output tmp buffer:
			for (size_t kkk=0; kkk<output.size(); ++kkk) {
				output[kkk] += outputTmp[kkk];
			}
			// TODO : if a signal is upper than 256* (for 24 bits) or 65335* (for 16 bits) the maximum of 1 it can create a real problem ...
		}
		RIVER_VERBOSE("    End stack process data ...");
		m_process.processIn(&output[0], _nbChunk, _outputBuffer, _nbChunk);
	} else if (muxerFormatType == audio::format_int32_on_int64) {
		//////////////////////////////////////////////////////////////////////////////////////////////////
		//            process 64 bits
		//////////////////////////////////////////////////////////////////////////////////////////////////
		etk::Vector<int64_t> output;
		RIVER_VERBOSE("resize=" << _nbChunk*m_process.getInputConfig().getMap().size());
		output.resize(_nbChunk*m_process.getInputConfig().getMap().size(), 0);
		const int64_t* outputTmp = null;
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (m_list[iii] == null) {
				continue;
			}
			if (m_list[iii]->getMode() != audio::river::modeInterface_output) {
				continue;
			}
			RIVER_VERBOSE("    IO name="<< m_list[iii]->getName() << " " << iii);
			// clear datas ...
			memset(&outputTmp2[0], 0, nbByteTmpBuffer);
			RIVER_VERBOSE("        request Data="<< _nbChunk << " time=" << _time);
			m_list[iii]->systemNeedOutputData(_time, &outputTmp2[0], _nbChunk, audio::getFormatBytes(muxerFormatType)*m_process.getInputConfig().getMap().size());
			outputTmp = reinterpret_cast<const int64_t*>(&outputTmp2[0]);
			RIVER_VERBOSE("        Mix it ...");
			// Add data to the output tmp buffer:
			for (size_t kkk=0; kkk<output.size(); ++kkk) {
				output[kkk] += outputTmp[kkk];
			}
			// TODO : if a signal is upper than 2000000000* the maximum of 1 it can create a real problem ...
		}
		RIVER_VERBOSE("    End stack process data ...");
		m_process.processIn(&output[0], _nbChunk, _outputBuffer, _nbChunk);
	} else if (muxerFormatType == audio::format_float) {
		//////////////////////////////////////////////////////////////////////////////////////////////////
		//            process 32 bits FLOAT
		//////////////////////////////////////////////////////////////////////////////////////////////////
		etk::Vector<float> output;
		RIVER_VERBOSE("resize=" << _nbChunk*m_process.getInputConfig().getMap().size());
		output.resize(_nbChunk*m_process.getInputConfig().getMap().size(), 0);
		const float* outputTmp = null;
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (m_list[iii] == null) {
				continue;
			}
			if (m_list[iii]->getMode() != audio::river::modeInterface_output) {
				continue;
			}
			RIVER_VERBOSE("    IO name="<< m_list[iii]->getName() << " " << iii);
			// clear datas ...
			memset(&outputTmp2[0], 0, nbByteTmpBuffer);
			RIVER_VERBOSE("        request Data="<< _nbChunk << " time=" << _time);
			m_list[iii]->systemNeedOutputData(_time, &outputTmp2[0], _nbChunk, audio::getFormatBytes(muxerFormatType)*m_process.getInputConfig().getMap().size());
			outputTmp = reinterpret_cast<const float*>(&outputTmp2[0]);
			RIVER_VERBOSE("        Mix it ...");
			// Add data to the output tmp buffer:
			for (size_t kkk=0; kkk<output.size(); ++kkk) {
				output[kkk] += outputTmp[kkk];
			}
		}
		RIVER_VERBOSE("    End stack process data ...");
		m_process.processIn(&output[0], _nbChunk, _outputBuffer, _nbChunk);
	} else if (muxerFormatType == audio::format_double) {
		//////////////////////////////////////////////////////////////////////////////////////////////////
		//            process 64 bits FLOAT
		//////////////////////////////////////////////////////////////////////////////////////////////////
		etk::Vector<double> output;
		RIVER_VERBOSE("resize=" << _nbChunk*m_process.getInputConfig().getMap().size());
		output.resize(_nbChunk*m_process.getInputConfig().getMap().size(), 0);
		const double* outputTmp = null;
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (m_list[iii] == null) {
				continue;
			}
			if (m_list[iii]->getMode() != audio::river::modeInterface_output) {
				continue;
			}
			RIVER_VERBOSE("    IO name="<< m_list[iii]->getName() << " " << iii);
			// clear datas ...
			memset(&outputTmp2[0], 0, nbByteTmpBuffer);
			RIVER_VERBOSE("        request Data="<< _nbChunk << " time=" << _time);
			m_list[iii]->systemNeedOutputData(_time, &outputTmp2[0], _nbChunk, audio::getFormatBytes(muxerFormatType)*m_process.getInputConfig().getMap().size());
			outputTmp = reinterpret_cast<const double*>(&outputTmp2[0]);
			RIVER_VERBOSE("        Mix it ...");
			// Add data to the output tmp buffer:
			for (size_t kkk=0; kkk<output.size(); ++kkk) {
				output[kkk] += outputTmp[kkk];
			}
		}
		RIVER_VERBOSE("    End stack process data ...");
		m_process.processIn(&output[0], _nbChunk, _outputBuffer, _nbChunk);
	} else {
		RIVER_ERROR("Wrong demuxer type: " << muxerFormatType);
		return;
	}
	// The feedback get the real output data (after processing ...==> then no nneed to specify for each channels
	RIVER_VERBOSE("    Feedback :");
	for (size_t iii=0; iii< m_list.size(); ++iii) {
		if (m_list[iii] == null) {
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

static void link(ememory::SharedPtr<etk::io::Interface>& _io, const etk::String& _first, const etk::String& _op, const etk::String& _second) {
	if (_op == "->") {
		*_io << "			" << _first << " -> " << _second << ";\n";
	} else if (_op == "<-") {
		*_io << "			" << _first << " -> " <<_second<< " [color=transparent];\n";
		*_io << "			" << _second << " -> " << _first << " [constraint=false];\n";
	}
}


void audio::river::io::Node::generateDot(ememory::SharedPtr<etk::io::Interface>& _io) {
	*_io << "	subgraph clusterNode_" << m_uid << " {\n";
	*_io << "		color=blue;\n";
	*_io << "		label=\"[" << m_uid << "] IO::Node : " << m_name << "\";\n";
	if (m_isInput == true) {
		*_io << "		node [shape=rarrow];\n";
		*_io << "			NODE_" << m_uid << "_HW_interface [ label=\"HW interface\\n interface=ALSA\\n stream=" << m_name << "\\n type=input\" ];\n";
		etk::String nameIn;
		etk::String nameOut;
		m_process.generateDotProcess(_io, 3, m_uid, nameIn, nameOut, false);
		*_io << "		node [shape=square];\n";
		*_io << "			NODE_" << m_uid << "_demuxer [ label=\"DEMUXER\\n format=" << etk::toString(m_process.getOutputConfig().getFormat()) << "\" ];\n";
		// Link all nodes :
		*_io << "			NODE_" << m_uid << "_HW_interface -> " << nameIn << " [arrowhead=\"open\"];\n";
		*_io << "			" << nameOut << " -> NODE_" << m_uid << "_demuxer [arrowhead=\"open\"];\n";
	} else {
		size_t nbOutput = getNumberOfInterfaceAvaillable(audio::river::modeInterface_output);
		size_t nbfeedback = getNumberOfInterfaceAvaillable(audio::river::modeInterface_feedback);
		*_io << "		node [shape=larrow];\n";
		*_io << "			NODE_" << m_uid << "_HW_interface [ label=\"HW interface\\n interface=ALSA\\n stream=" << m_name << "\\n type=output\" ];\n";
		etk::String nameIn;
		etk::String nameOut;
		if (nbOutput>0) {
			m_process.generateDotProcess(_io, 3, m_uid, nameIn, nameOut, true);
		}
		*_io << "		node [shape=square];\n";
		if (nbOutput>0) {
			*_io << "			NODE_" << m_uid << "_muxer [ label=\"MUXER\\n format=" << etk::toString(m_process.getInputConfig().getFormat()) << "\" ];\n";
		}
		if (nbfeedback>0) {
			*_io << "			NODE_" << m_uid << "_demuxer [ label=\"DEMUXER\\n format=" << etk::toString(m_process.getOutputConfig().getFormat()) << "\" ];\n";
		}
		// Link all nodes :
		if (nbOutput>0) {
			link(_io, "NODE_" + etk::toString(m_uid) + "_HW_interface", "<-", nameOut);
			link(_io, nameIn, "<-", "NODE_" + etk::toString(m_uid) + "_muxer");
		}
		if (nbfeedback>0) {
			*_io << "			NODE_" << m_uid << "_HW_interface -> NODE_" << m_uid << "_demuxer [arrowhead=\"open\"];\n";
		}
		if (    nbOutput>0
		     && nbfeedback>0) {
			*_io << "			{ rank=same; NODE_" << m_uid << "_demuxer; NODE_" << m_uid << "_muxer }\n";
		}
		
	}
	*_io << "	}\n	\n";
	
	for (size_t iii=0; iii< m_listAvaillable.size(); ++iii) {
		if (m_listAvaillable[iii].expired() == true) {
			continue;
		}
		ememory::SharedPtr<audio::river::Interface> element = m_listAvaillable[iii].lock();
		if (element == null) {
			continue;
		}
		bool isLink = false;
		for (size_t jjj=0; jjj<m_list.size(); ++jjj) {
			if (element == m_list[jjj]) {
				isLink = true;
			}
		}
		if (element != null) {
			if (element->getMode() == modeInterface_input) {
				element->generateDot(_io, "NODE_" + etk::toString(m_uid) + "_demuxer", isLink);
			} else if (element->getMode() == modeInterface_output) {
				element->generateDot(_io, "NODE_" + etk::toString(m_uid) + "_muxer", isLink);
			} else if (element->getMode() == modeInterface_feedback) {
				element->generateDot(_io, "NODE_" + etk::toString(m_uid) + "_demuxer", isLink);
			} else {
				
			}
		}
	}
}


void audio::river::io::Node::startInGroup() {
	ememory::SharedPtr<audio::river::io::Group> group = m_group.lock();
	if (group != null) {
		group->start();
	} else {
		start();
	}
}

void audio::river::io::Node::stopInGroup() {
	ememory::SharedPtr<audio::river::io::Group> group = m_group.lock();
	if (group != null) {
		group->stop();
	} else {
		stop();
	}
}
