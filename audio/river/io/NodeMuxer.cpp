/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <audio/river/io/NodeMuxer.h>
#include <audio/river/debug.h>
#include <etk/types.h>
#include <ememory/memory.h>
#include <functional>

ememory::SharedPtr<audio::river::io::NodeMuxer> audio::river::io::NodeMuxer::create(const std::string& _name, const ejson::Object& _config) {
	return ememory::SharedPtr<audio::river::io::NodeMuxer>(new audio::river::io::NodeMuxer(_name, _config));
}

ememory::SharedPtr<audio::river::Interface> audio::river::io::NodeMuxer::createInput(float _freq,
                                                                      const std::vector<audio::channel>& _map,
                                                                      audio::format _format,
                                                                      const std::string& _objectName,
                                                                      const std::string& _name) {
	// check if the output exist
	const ejson::Object tmppp = m_config[_objectName].toObject();
	if (tmppp.exist() == false) {
		RIVER_ERROR("can not open a non existance virtual interface: '" << _objectName << "' not present in : " << m_config.getKeys());
		return ememory::SharedPtr<audio::river::Interface>();
	}
	std::string streamName = tmppp["map-on"].toString().get("error");
	
	
	// check if it is an Output:
	std::string type = tmppp["io"].toString().get("error");
	if (    type != "input"
	     && type != "feedback") {
		RIVER_ERROR("can not open in output a virtual interface: '" << streamName << "' configured has : " << type);
		return ememory::SharedPtr<audio::river::Interface>();
	}
	// get global hardware interface:
	ememory::SharedPtr<audio::river::io::Manager> manager = audio::river::io::Manager::getInstance();
	// get the output or input channel :
	ememory::SharedPtr<audio::river::io::Node> node = manager->getNode(streamName);
	// create user iterface:
	ememory::SharedPtr<audio::river::Interface> interface;
	interface = audio::river::Interface::create(_freq, _map, _format, node, tmppp);
	if (interface != nullptr) {
		interface->setName(_name);
	}
	return interface;
}


audio::river::io::NodeMuxer::NodeMuxer(const std::string& _name, const ejson::Object& _config) :
  Node(_name, _config) {
	audio::drain::IOFormatInterface interfaceFormat = getInterfaceFormat();
	audio::drain::IOFormatInterface hardwareFormat = getHarwareFormat();
	m_sampleTime = audio::Duration(1000000000/int64_t(hardwareFormat.getFrequency()));
	/**
		# connect in input mode
		map-on-input-1:{
			# generic virtual definition
			io:"input",
			map-on:"microphone",
			resampling-type:"speexdsp",
			resampling-option:"quality=10"
		},
		# connect in feedback mode
		map-on-input-2:{
			io:"feedback",
			map-on:"speaker",
			resampling-type:"speexdsp",
			resampling-option:"quality=10",
		},
		input-2-remap:["rear-left", "rear-right"], # remap the IO inputs ...
		# AEC algo definition
		algo:"river-remover",
		algo-mode:"cutter",
	*/
	RIVER_INFO("Create IN 1 : ");
	m_interfaceInput1 = createInput(hardwareFormat.getFrequency(),
	                                std::vector<audio::channel>(),
	                                hardwareFormat.getFormat(),
	                                "map-on-input-1",
	                                _name + "-muxer-in1");
	if (m_interfaceInput1 == nullptr) {
		RIVER_ERROR("Can not opne virtual device ... map-on-input-1 in " << _name);
		return;
	}
	const ejson::Array listChannelMap = m_config["input-1-remap"].toArray();
	if (    listChannelMap.exist() == false
	     || listChannelMap.size() == 0) {
		m_mapInput1 = m_interfaceInput1->getInterfaceFormat().getMap();
	} else {
		m_mapInput1.clear();
		for (const auto it : listChannelMap) {
			std::string value = it.toString().get();
			m_mapInput1.push_back(audio::getChannelFromString(value));
		}
		if (m_mapInput1.size() != m_interfaceInput1->getInterfaceFormat().getMap().size()) {
			RIVER_ERROR("Request remap of the Input 1 the 2 size is wrong ... request=");
			m_mapInput1 = m_interfaceInput1->getInterfaceFormat().getMap();
		}
	}
	
	RIVER_INFO("Create IN 2 : ");
	m_interfaceInput2 = createInput(hardwareFormat.getFrequency(),
	                                std::vector<audio::channel>(),
	                                hardwareFormat.getFormat(),
	                                "map-on-input-2",
	                                _name + "-muxer-in2");
	if (m_interfaceInput2 == nullptr) {
		RIVER_ERROR("Can not opne virtual device ... map-on-input-2 in " << _name);
		return;
	}
	const ejson::Array listChannelMap2 = m_config["input-2-remap"].toArray();
	if (    listChannelMap2.exist() == false
	     || listChannelMap2.size() == 0) {
		m_mapInput2 = m_interfaceInput2->getInterfaceFormat().getMap();
	} else {
		m_mapInput2.clear();
		for (const auto it : listChannelMap2) {
			std::string value = it.toString().get();
			m_mapInput2.push_back(audio::getChannelFromString(value));
		}
		if (m_mapInput2.size() != m_interfaceInput2->getInterfaceFormat().getMap().size()) {
			RIVER_ERROR("Request remap of the Input 2 the 2 size is wrong ... request=");
			m_mapInput2 = m_interfaceInput2->getInterfaceFormat().getMap();
		}
	}
	
	// set callback mode ...
	m_interfaceInput1->setInputCallback(std::bind(&audio::river::io::NodeMuxer::onDataReceivedInput1,
	                                                this,
	                                                std::placeholders::_1,
	                                                std::placeholders::_2,
	                                                std::placeholders::_3,
	                                                std::placeholders::_4,
	                                                std::placeholders::_5,
	                                                std::placeholders::_6));
	// set callback mode ...
	m_interfaceInput2->setInputCallback(std::bind(&audio::river::io::NodeMuxer::onDataReceivedInput2,
	                                                this,
	                                                std::placeholders::_1,
	                                                std::placeholders::_2,
	                                                std::placeholders::_3,
	                                                std::placeholders::_4,
	                                                std::placeholders::_5,
	                                                std::placeholders::_6));
	
	m_bufferInput1.setCapacity(std::chrono::milliseconds(1000),
	                           audio::getFormatBytes(hardwareFormat.getFormat())*m_mapInput1.size(),
	                           hardwareFormat.getFrequency());
	m_bufferInput2.setCapacity(std::chrono::milliseconds(1000),
	                           audio::getFormatBytes(hardwareFormat.getFormat())*m_mapInput2.size(),
	                           hardwareFormat.getFrequency());
	
	m_process.updateInterAlgo();
}

audio::river::io::NodeMuxer::~NodeMuxer() {
	RIVER_INFO("close input stream");
	stop();
	m_interfaceInput1.reset();
	m_interfaceInput2.reset();
};

void audio::river::io::NodeMuxer::start() {
	std::unique_lock<std::mutex> lock(m_mutex);
	RIVER_INFO("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
	if (m_interfaceInput1 != nullptr) {
		RIVER_INFO("Start FEEDBACK : ");
		m_interfaceInput1->start();
	}
	if (m_interfaceInput2 != nullptr) {
		RIVER_INFO("Start Microphone : ");
		m_interfaceInput2->start();
	}
}

void audio::river::io::NodeMuxer::stop() {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_interfaceInput1 != nullptr) {
		m_interfaceInput1->stop();
	}
	if (m_interfaceInput2 != nullptr) {
		m_interfaceInput2->stop();
	}
}


void audio::river::io::NodeMuxer::onDataReceivedInput1(const void* _data,
                                                const audio::Time& _time,
                                                size_t _nbChunk,
                                                enum audio::format _format,
                                                uint32_t _frequency,
                                                const std::vector<audio::channel>& _map) {
	RIVER_DEBUG("Microphone Time=" << _time << " _nbChunk=" << _nbChunk << " _map=" << _map << " _format=" << _format << " freq=" << _frequency);
	RIVER_DEBUG("           next=" << _time + audio::Duration(0, _nbChunk*1000000000LL/int64_t(_frequency)) );
	if (_format != audio::format_int16) {
		RIVER_ERROR("call wrong type ... (need int16_t)");
	}
	// push data synchronize
	std::unique_lock<std::mutex> lock(m_mutex);
	m_bufferInput1.write(_data, _nbChunk, _time);
	//RIVER_SAVE_FILE_MACRO(int16_t, "REC_Microphone.raw", _data, _nbChunk*_map.size());
	process();
}

void audio::river::io::NodeMuxer::onDataReceivedInput2(const void* _data,
                                                const audio::Time& _time,
                                                size_t _nbChunk,
                                                enum audio::format _format,
                                                uint32_t _frequency,
                                                const std::vector<audio::channel>& _map) {
	RIVER_DEBUG("FeedBack   Time=" << _time << " _nbChunk=" << _nbChunk << " _map=" << _map << " _format=" << _format << " freq=" << _frequency);
	RIVER_DEBUG("           next=" << _time + audio::Duration(0, _nbChunk*1000000000LL/int64_t(_frequency)) );
	if (_format != audio::format_int16) {
		RIVER_ERROR("call wrong type ... (need int16_t)");
	}
	// push data synchronize
	std::unique_lock<std::mutex> lock(m_mutex);
	m_bufferInput2.write(_data, _nbChunk, _time);
	//RIVER_SAVE_FILE_MACRO(int16_t, "REC_FeedBack.raw", _data, _nbChunk*_map.size());
	process();
}

void audio::river::io::NodeMuxer::process() {
	if (m_bufferInput1.getSize() <= 256) {
		return;
	}
	if (m_bufferInput2.getSize() <= 256) {
		return;
	}
	audio::Time in1Time = m_bufferInput1.getReadTimeStamp();
	audio::Time in2Time = m_bufferInput2.getReadTimeStamp();
	audio::Duration delta;
	if (in1Time < in2Time) {
		delta = in2Time - in1Time;
	} else {
		delta = in1Time - in2Time;
	}
	
	RIVER_INFO("check delta " << delta.count() << " > " << m_sampleTime.count());
	if (delta > m_sampleTime) {
		// Synchronize if possible
		if (in1Time < in2Time) {
			RIVER_INFO("in1Time < in2Time : Change Microphone time start " << in2Time);
			RIVER_INFO("                                 old time stamp=" << m_bufferInput1.getReadTimeStamp());
			m_bufferInput1.setReadPosition(in2Time);
			RIVER_INFO("                                 new time stamp=" << m_bufferInput1.getReadTimeStamp());
		}
		if (in1Time > in2Time) {
			RIVER_INFO("in1Time > in2Time : Change FeedBack time start " << in1Time);
			RIVER_INFO("                               old time stamp=" << m_bufferInput2.getReadTimeStamp());
			m_bufferInput2.setReadPosition(in1Time);
			RIVER_INFO("                               new time stamp=" << m_bufferInput2.getReadTimeStamp());
		}
	}
	// check if enought time after synchronisation ...
	if (m_bufferInput1.getSize() <= 256) {
		return;
	}
	if (m_bufferInput2.getSize() <= 256) {
		return;
	}
	
	in1Time = m_bufferInput1.getReadTimeStamp();
	in2Time = m_bufferInput2.getReadTimeStamp();
	
	if (in1Time-in2Time > m_sampleTime) {
		RIVER_ERROR("Can not synchronize flow ... : " << in1Time << " != " << in2Time << "  delta = " << (in1Time-in2Time).count()/1000 << " µs");
		return;
	}
	std::vector<uint8_t> dataIn1;
	std::vector<uint8_t> dataIn2;
	dataIn1.resize(256*sizeof(int16_t)*m_mapInput1.size(), 0);
	dataIn2.resize(256*sizeof(int16_t)*m_mapInput2.size(), 0);
	m_data.resize(256*sizeof(int16_t)*getInterfaceFormat().getMap().size(), 0);
	while (true) {
		in1Time = m_bufferInput1.getReadTimeStamp();
		in2Time = m_bufferInput2.getReadTimeStamp();
		//RIVER_INFO(" process 256 samples ... in1Time=" << in1Time << " in2Time=" << in2Time << " delta = " << (in1Time-in2Time).count());
		m_bufferInput1.read(&dataIn1[0], 256);
		m_bufferInput2.read(&dataIn2[0], 256);
		//RIVER_SAVE_FILE_MACRO(int16_t, "REC_INPUT1.raw", &dataIn1[0], 256 * m_mapInput1.size());
		//RIVER_SAVE_FILE_MACRO(int16_t, "REC_INPUT2.raw", &dataIn2[0], 256 * m_mapInput2.size());
		// if threaded : send event / otherwise, process ...
		processMuxer(&dataIn1[0], &dataIn2[0], 256, in1Time);
		if (    m_bufferInput1.getSize() <= 256
		     || m_bufferInput2.getSize() <= 256) {
			return;
		}
	}
}


void audio::river::io::NodeMuxer::reorder(void* _output, uint32_t _nbChunk, void* _input, const std::vector<audio::channel>& _mapInput) {
	// real process: (only depend of data size):
	switch (getInterfaceFormat().getFormat()) {
		case audio::format_int8:
			{
				RIVER_VERBOSE("convert " << _mapInput << " ==> " << getInterfaceFormat().getMap());
				int8_t* in = static_cast<int8_t*>(_input);
				int8_t* out = static_cast<int8_t*>(_output);
				for (size_t kkk=0; kkk<getInterfaceFormat().getMap().size(); ++kkk) {
					int32_t convertId = -1;
					if (    _mapInput.size() == 1
					     && _mapInput[0] == audio::channel_frontCenter) {
						convertId = 0;
					} else {
						for (size_t jjj=0; jjj<_mapInput.size(); ++jjj) {
							if (getInterfaceFormat().getMap()[kkk] == _mapInput[jjj]) {
								convertId = jjj;
								break;
							}
						}
					}
					RIVER_VERBOSE("    " << convertId << " ==> " << kkk);
					if (convertId != -1) {
						for (size_t iii=0; iii<_nbChunk; ++iii) {
							out[iii*getInterfaceFormat().getMap().size()+kkk] = in[iii*_mapInput.size()+convertId];
						}
					}
				}
			}
			break;
		default:
		case audio::format_int16:
			if (getInterfaceFormat().getMap().size() == 1) {
				RIVER_VERBOSE("convert " << _mapInput << " ==> " << getInterfaceFormat().getMap());
				int16_t* in = static_cast<int16_t*>(_input);
				int16_t* out = static_cast<int16_t*>(_output);
				for (size_t iii=0; iii<_nbChunk; ++iii) {
					int32_t val = 0;
					for (size_t jjj=0; jjj<_mapInput.size(); ++jjj) {
						val += in[iii*_mapInput.size()+jjj];
					}
					out[iii] = val/_mapInput.size();
				}
			} else {
				RIVER_VERBOSE("convert " << _mapInput << " ==> " << getInterfaceFormat().getMap());
				int16_t* in = static_cast<int16_t*>(_input);
				int16_t* out = static_cast<int16_t*>(_output);
				for (size_t kkk=0; kkk<getInterfaceFormat().getMap().size(); ++kkk) {
					int32_t convertId = -1;
					if (    _mapInput.size() == 1
					     && _mapInput[0] == audio::channel_frontCenter) {
						convertId = 0;
					} else {
						for (size_t jjj=0; jjj<_mapInput.size(); ++jjj) {
							if (getInterfaceFormat().getMap()[kkk] == _mapInput[jjj]) {
								convertId = jjj;
								break;
							}
						}
					}
					RIVER_VERBOSE("    " << convertId << " ==> " << kkk);
					if (convertId != -1) {
						for (size_t iii=0; iii<_nbChunk; ++iii) {
							out[iii*getInterfaceFormat().getMap().size()+kkk] = in[iii*_mapInput.size()+convertId];
						}
					}
				}
			}
			break;
		case audio::format_int16_on_int32:
		case audio::format_int24:
		case audio::format_int32:
		case audio::format_float:
			{
				RIVER_VERBOSE("convert (2) " << _mapInput << " ==> " << getInterfaceFormat().getMap());
				uint32_t* in = static_cast<uint32_t*>(_input);
				uint32_t* out = static_cast<uint32_t*>(_output);
				for (size_t kkk=0; kkk<getInterfaceFormat().getMap().size(); ++kkk) {
					int32_t convertId = -1;
					if (    _mapInput.size() == 1
					     && _mapInput[0] == audio::channel_frontCenter) {
						convertId = 0;
					} else {
						for (size_t jjj=0; jjj<_mapInput.size(); ++jjj) {
							if (getInterfaceFormat().getMap()[kkk] == _mapInput[jjj]) {
								convertId = jjj;
								break;
							}
						}
					}
					if (convertId != -1) {
						for (size_t iii=0; iii<_nbChunk; ++iii) {
							out[iii*getInterfaceFormat().getMap().size()+kkk] = in[iii*_mapInput.size()+convertId];
						}
					}
				}
			}
			break;
		case audio::format_double:
			{
				RIVER_VERBOSE("convert (2) " << _mapInput << " ==> " << getInterfaceFormat().getMap());
				uint64_t* in = static_cast<uint64_t*>(_input);
				uint64_t* out = static_cast<uint64_t*>(_output);
				for (size_t kkk=0; kkk<getInterfaceFormat().getMap().size(); ++kkk) {
					int32_t convertId = -1;
					if (    _mapInput.size() == 1
					     && _mapInput[0] == audio::channel_frontCenter) {
						convertId = 0;
					} else {
						for (size_t jjj=0; jjj<_mapInput.size(); ++jjj) {
							if (getInterfaceFormat().getMap()[kkk] == _mapInput[jjj]) {
								convertId = jjj;
								break;
							}
						}
					}
					if (convertId != -1) {
						for (size_t iii=0; iii<_nbChunk; ++iii) {
							out[iii*getInterfaceFormat().getMap().size()+kkk] = in[iii*_mapInput.size()+convertId];
						}
					}
				}
			}
			break;
	}
}

void audio::river::io::NodeMuxer::processMuxer(void* _dataIn1, void* _dataIn2, uint32_t _nbChunk, const audio::Time& _time) {
	//RIVER_INFO("must Mux data : " << m_mapInput1 << " + " << m_mapInput2 << " ==> " << getInterfaceFormat().getMap());
	memset(&m_data[0], 0, m_data.size());
	reorder(&m_data[0], _nbChunk, _dataIn1, m_mapInput1);
	reorder(&m_data[0], _nbChunk, _dataIn2, m_mapInput2);
	newInput(&m_data[0], _nbChunk, _time);
}


void audio::river::io::NodeMuxer::generateDot(etk::FSNode& _node) {
	_node << "	subgraph clusterNode_" << m_uid << " {\n";
	_node << "		color=blue;\n";
	_node << "		label=\"[" << m_uid << "] IO::Node : " << m_name << "\";\n";

		_node << "		node [shape=box];\n";
		// TODO : Create a structure ...
		_node << "			NODE_" << m_uid << "_HW_MUXER [ label=\"Muxer\\n channelMap=" << etk::to_string(getInterfaceFormat().getMap()) << "\" ];\n";
		std::string nameIn;
		std::string nameOut;
		m_process.generateDot(_node, 3, m_uid, nameIn, nameOut, false);
		_node << "		node [shape=square];\n";
		_node << "			NODE_" << m_uid << "_demuxer [ label=\"DEMUXER\\n format=" << etk::to_string(m_process.getOutputConfig().getFormat()) << "\" ];\n";
		// Link all nodes :
		_node << "			NODE_" << m_uid << "_HW_MUXER -> " << nameIn << ";\n";
		_node << "			" << nameOut << " -> NODE_" << m_uid << "_demuxer;\n";
	_node << "	}\n";
	if (m_interfaceInput2 != nullptr) {
		_node << "	" << m_interfaceInput2->getDotNodeName() << " -> NODE_" << m_uid << "_HW_MUXER;\n";
	}
	if (m_interfaceInput1 != nullptr) {
		_node << "	" << m_interfaceInput1->getDotNodeName() << " -> NODE_" << m_uid << "_HW_MUXER;\n";
	}
	_node << "	\n";
	for (size_t iii=0; iii< m_listAvaillable.size(); ++iii) {
		if (m_listAvaillable[iii].expired() == true) {
			continue;
		}
		ememory::SharedPtr<audio::river::Interface> element = m_listAvaillable[iii].lock();
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
	_node << "\n";
}
