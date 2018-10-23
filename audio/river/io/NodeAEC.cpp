/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <audio/river/io/NodeAEC.hpp>
#include <audio/river/debug.hpp>
#include <etk/types.hpp>
#include <ememory/memory.hpp>
#include <etk/Function.hpp>

ememory::SharedPtr<audio::river::io::NodeAEC> audio::river::io::NodeAEC::create(const etk::String& _name, const ejson::Object& _config) {
	return ememory::SharedPtr<audio::river::io::NodeAEC>(ETK_NEW(audio::river::io::NodeAEC, _name, _config));
}

ememory::SharedPtr<audio::river::Interface> audio::river::io::NodeAEC::createInput(float _freq,
                                                                    const etk::Vector<audio::channel>& _map,
                                                                    audio::format _format,
                                                                    const etk::String& _objectName,
                                                                    const etk::String& _name) {
	// check if the output exist
	const ejson::Object tmppp = m_config[_objectName].toObject();
	if (tmppp.exist() == false) {
		RIVER_ERROR("can not open a non existance virtual interface: '" << _objectName << "' not present in : " << m_config.getKeys());
		return ememory::SharedPtr<audio::river::Interface>();
	}
	etk::String streamName = tmppp["map-on"].toString().get("error");
	
	m_nbChunk = m_config["nb-chunk"].toNumber().get(1024);
	// check if it is an Output:
	etk::String type = tmppp["io"].toString().get("error");
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
	if (interface != null) {
		interface->setName(_name);
	}
	return interface;
}


audio::river::io::NodeAEC::NodeAEC(const etk::String& _name, const ejson::Object& _config) :
  Node(_name, _config),
  m_P_attaqueTime(1),
  m_P_releaseTime(100),
  m_P_minimumGain(10),
  m_P_threshold(2),
  m_P_latencyTime(100) {
	audio::drain::IOFormatInterface interfaceFormat = getInterfaceFormat();
	audio::drain::IOFormatInterface hardwareFormat = getHarwareFormat();
	m_sampleTime = audio::Duration(1000000000/int64_t(hardwareFormat.getFrequency()));
	/**
		# connect in input mode
		map-on-microphone:{
			# generic virtual definition
			io:"input",
			map-on:"microphone",
			resampling-type:"speexdsp",
			resampling-option:"quality=10"
		},
		# connect in feedback mode
		map-on-feedback:{
			io:"feedback",
			map-on:"speaker",
			resampling-type:"speexdsp",
			resampling-option:"quality=10",
		},
		# AEC algo definition
		algo:"river-remover",
		algo-mode:"cutter",
	*/
	etk::Vector<audio::channel> feedbackMap;
	feedbackMap.pushBack(audio::channel_frontCenter);
	RIVER_INFO("Create FEEDBACK : ");
	m_interfaceFeedBack = createInput(hardwareFormat.getFrequency(),
	                                  feedbackMap,
	                                  hardwareFormat.getFormat(),
	                                  "map-on-feedback",
	                                  _name + "-AEC-feedback");
	if (m_interfaceFeedBack == null) {
		RIVER_ERROR("Can not opne virtual device ... map-on-feedback in " << _name);
		return;
	}
	RIVER_INFO("Create MICROPHONE : ");
	m_interfaceMicrophone = createInput(hardwareFormat.getFrequency(),
	                                    hardwareFormat.getMap(),
	                                    hardwareFormat.getFormat(),
	                                    "map-on-microphone",
	                                    _name + "-AEC-microphone");
	if (m_interfaceMicrophone == null) {
		RIVER_ERROR("Can not opne virtual device ... map-on-microphone in " << _name);
		return;
	}
	
	// set callback mode ...
	m_interfaceFeedBack->setInputCallback([=](const void* _data,
	                                          const audio::Time& _time,
	                                          size_t _nbChunk,
	                                          enum audio::format _format,
	                                          uint32_t _frequency,
	                                          const etk::Vector<audio::channel>& _map) {
	                                          	onDataReceivedFeedBack(_data, _time, _nbChunk, _format, _frequency, _map);
	                                          });
	// set callback mode ...
	m_interfaceMicrophone->setInputCallback([=](const void* _data,
	                                            const audio::Time& _time,
	                                            size_t _nbChunk,
	                                            enum audio::format _format,
	                                            uint32_t _frequency,
	                                            const etk::Vector<audio::channel>& _map) {
	                                            	onDataReceivedMicrophone(_data, _time, _nbChunk, _format, _frequency, _map);
	                                            });
	m_bufferMicrophone.setCapacity(echrono::milliseconds(1000),
	                               audio::getFormatBytes(hardwareFormat.getFormat())*hardwareFormat.getMap().size(),
	                               hardwareFormat.getFrequency());
	m_bufferFeedBack.setCapacity(echrono::milliseconds(1000),
	                             audio::getFormatBytes(hardwareFormat.getFormat()), // only one channel ...
	                             hardwareFormat.getFrequency());
	
	m_process.updateInterAlgo();
}

audio::river::io::NodeAEC::~NodeAEC() {
	RIVER_INFO("close input stream");
	stop();
	m_interfaceFeedBack.reset();
	m_interfaceMicrophone.reset();
};

void audio::river::io::NodeAEC::start() {
	ethread::UniqueLock lock(m_mutex);
	RIVER_INFO("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
	if (m_interfaceFeedBack != null) {
		RIVER_INFO("Start FEEDBACK : ");
		m_interfaceFeedBack->start();
	}
	if (m_interfaceMicrophone != null) {
		RIVER_INFO("Start Microphone : ");
		m_interfaceMicrophone->start();
	}
}

void audio::river::io::NodeAEC::stop() {
	ethread::UniqueLock lock(m_mutex);
	if (m_interfaceFeedBack != null) {
		m_interfaceFeedBack->stop();
	}
	if (m_interfaceMicrophone != null) {
		m_interfaceMicrophone->stop();
	}
}


void audio::river::io::NodeAEC::onDataReceivedMicrophone(const void* _data,
                                                  const audio::Time& _time,
                                                  size_t _nbChunk,
                                                  enum audio::format _format,
                                                  uint32_t _frequency,
                                                  const etk::Vector<audio::channel>& _map) {
	RIVER_DEBUG("Microphone Time=" << _time << " _nbChunk=" << _nbChunk << " _map=" << _map << " _format=" << _format << " freq=" << _frequency);
	RIVER_DEBUG("           next=" << _time + audio::Duration(0, _nbChunk*1000000000LL/int64_t(_frequency)) );
	if (_format != audio::format_int16) {
		RIVER_ERROR("call wrong type ... (need int16_t)");
	}
	// push data synchronize
	ethread::UniqueLock lock(m_mutex);
	m_bufferMicrophone.write(_data, _nbChunk, _time);
	//RIVER_SAVE_FILE_MACRO(int16_t, "REC_Microphone.raw", _data, _nbChunk*_map.size());
	process();
}

void audio::river::io::NodeAEC::onDataReceivedFeedBack(const void* _data,
                                                const audio::Time& _time,
                                                size_t _nbChunk,
                                                enum audio::format _format,
                                                uint32_t _frequency,
                                                const etk::Vector<audio::channel>& _map) {
	RIVER_DEBUG("FeedBack   Time=" << _time << " _nbChunk=" << _nbChunk << " _map=" << _map << " _format=" << _format << " freq=" << _frequency);
	RIVER_DEBUG("           next=" << _time + audio::Duration(0, _nbChunk*1000000000LL/int64_t(_frequency)) );
	if (_format != audio::format_int16) {
		RIVER_ERROR("call wrong type ... (need int16_t)");
	}
	// push data synchronize
	ethread::UniqueLock lock(m_mutex);
	m_bufferFeedBack.write(_data, _nbChunk, _time);
	//RIVER_SAVE_FILE_MACRO(int16_t, "REC_FeedBack.raw", _data, _nbChunk*_map.size());
	process();
}

void audio::river::io::NodeAEC::process() {
	if (    m_bufferMicrophone.getSize() <= m_nbChunk
	     || m_bufferFeedBack.getSize() <= m_nbChunk) {
		return;
	}
	audio::Time MicTime = m_bufferMicrophone.getReadTimeStamp();
	audio::Time fbTime = m_bufferFeedBack.getReadTimeStamp();
	audio::Duration delta;
	if (MicTime < fbTime) {
		delta = fbTime - MicTime;
	} else {
		delta = MicTime - fbTime;
	}
	
	RIVER_INFO("check delta " << delta << " > " << m_sampleTime);
	if (delta > m_sampleTime) {
		// Synchronize if possible
		if (MicTime < fbTime) {
			RIVER_INFO("micTime < fbTime : Change Microphone time start " << fbTime);
			RIVER_INFO("                                 old time stamp=" << m_bufferMicrophone.getReadTimeStamp());
			m_bufferMicrophone.setReadPosition(fbTime);
			RIVER_INFO("                                 new time stamp=" << m_bufferMicrophone.getReadTimeStamp());
		}
		if (MicTime > fbTime) {
			RIVER_INFO("micTime > fbTime : Change FeedBack time start " << MicTime);
			RIVER_INFO("                               old time stamp=" << m_bufferFeedBack.getReadTimeStamp());
			m_bufferFeedBack.setReadPosition(MicTime);
			RIVER_INFO("                               new time stamp=" << m_bufferFeedBack.getReadTimeStamp());
		}
	}
	// check if enought time after synchronisation ...
	if (    m_bufferMicrophone.getSize() <= m_nbChunk
	     || m_bufferFeedBack.getSize() <= m_nbChunk) {
		return;
	}
	
	MicTime = m_bufferMicrophone.getReadTimeStamp();
	fbTime = m_bufferFeedBack.getReadTimeStamp();
	
	if (MicTime-fbTime > m_sampleTime) {
		RIVER_ERROR("Can not synchronize flow ... : " << MicTime << " != " << fbTime << "  delta = " << (MicTime-fbTime));
		return;
	}
	etk::Vector<uint8_t> dataMic;
	etk::Vector<uint8_t> dataFB;
	dataMic.resize(m_nbChunk*sizeof(int16_t)*2, 0);
	dataFB.resize(m_nbChunk*sizeof(int16_t), 0);
	while (true) {
		MicTime = m_bufferMicrophone.getReadTimeStamp();
		fbTime = m_bufferFeedBack.getReadTimeStamp();
		RIVER_INFO(" process 256 samples ... micTime=" << MicTime << " fbTime=" << fbTime << " delta = " << (MicTime-fbTime));
		m_bufferMicrophone.read(&dataMic[0], m_nbChunk);
		m_bufferFeedBack.read(&dataFB[0], m_nbChunk);
		RIVER_SAVE_FILE_MACRO(int16_t, "REC_Microphone_sync.raw", &dataMic[0], m_nbChunk*getHarwareFormat().getMap().size());
		RIVER_SAVE_FILE_MACRO(int16_t, "REC_FeedBack_sync.raw", &dataFB[0], m_nbChunk);
		// if threaded : send event / otherwise, process ...
		processAEC(&dataMic[0], &dataFB[0], m_nbChunk, MicTime);
		if (    m_bufferMicrophone.getSize() <= m_nbChunk
		     || m_bufferFeedBack.getSize() <= m_nbChunk) {
			return;
		}
	}
}


void audio::river::io::NodeAEC::processAEC(void* _dataMic, void* _dataFB, uint32_t _nbChunk, const audio::Time& _time) {
	audio::drain::IOFormatInterface hardwareFormat = getHarwareFormat();
	// TODO : Set all these parameter in the parameter configuration section ...
	int32_t attaqueTime = etk::min(etk::max(0,m_P_attaqueTime),1000);
	int32_t releaseTime = etk::min(etk::max(0,m_P_releaseTime),1000);
	int32_t min_gain = 32767 * etk::min(etk::max(0,m_P_minimumGain),1000) / 1000;
	int32_t threshold = 32767 * etk::min(etk::max(0,m_P_threshold),1000) / 1000;
	
	int32_t latencyTime = etk::min(etk::max(0,m_P_latencyTime),1000);
	int32_t nb_sample_latency = (hardwareFormat.getFrequency()/1000)*latencyTime;
	
	int32_t increaseSample = 32767;
	if (attaqueTime != 0) {
		increaseSample = 32767/(hardwareFormat.getFrequency() * attaqueTime / 1000);
	}
	int32_t decreaseSample = 32767;
	if (attaqueTime != 0) {
		decreaseSample = 32767/(hardwareFormat.getFrequency() * releaseTime / 1000);
	}
	// Process section:
	int16_t* dataMic = static_cast<int16_t*>(_dataMic);
	int16_t* dataFB = static_cast<int16_t*>(_dataFB);
	for (size_t iii=0; iii<_nbChunk; ++iii) {
		if (abs(*dataFB++) > threshold) {
			m_sampleCount = 0;
		} else {
			m_sampleCount++;
		}
		if (m_sampleCount > nb_sample_latency) {
			m_gainValue += decreaseSample;
			if (m_gainValue >= 32767) {
				m_gainValue = 32767;
			}
		} else {
			if (m_gainValue <= increaseSample) {
				m_gainValue = 0;
			} else {
				m_gainValue -= increaseSample;
			}
			if (m_gainValue < min_gain) {
				m_gainValue = min_gain;
			}
		}
		for (size_t jjj=0; jjj<hardwareFormat.getMap().size(); ++jjj) {
			*dataMic = static_cast<int16_t>((*dataMic * m_gainValue) >> 15);
			dataMic++;
		}
	}
		RIVER_SAVE_FILE_MACRO(int16_t, "REC_Microphone_clean.raw", _dataMic, _nbChunk*getHarwareFormat().getMap().size());
	// simply send to upper requester...
	newInput(_dataMic, _nbChunk, _time);
}


void audio::river::io::NodeAEC::generateDot(ememory::SharedPtr<etk::io::Interface>& _io) {
	*_io << "	subgraph clusterNode_" << m_uid << " {\n";
	*_io << "		color=blue;\n";
	*_io << "		label=\"[" << m_uid << "] IO::Node : " << m_name << "\";\n";
		*_io << "			NODE_" << m_uid << "_HW_AEC [ label=\"AEC\\n channelMap=" << etk::toString(getInterfaceFormat().getMap()) << "\" ];\n";
		etk::String nameIn;
		etk::String nameOut;
		m_process.generateDot(_io, 3, m_uid, nameIn, nameOut, false);
		*_io << "		node [shape=square];\n";
		*_io << "			NODE_" << m_uid << "_demuxer [ label=\"DEMUXER\\n format=" << etk::toString(m_process.getOutputConfig().getFormat()) << "\" ];\n";
		// Link all nodes :
		*_io << "			NODE_" << m_uid << "_HW_AEC -> " << nameIn << ";\n";
		*_io << "			" << nameOut << " -> NODE_" << m_uid << "_demuxer;\n";
	*_io << "	}\n";
	if (m_interfaceMicrophone != null) {
		*_io << "	" << m_interfaceMicrophone->getDotNodeName() << " -> NODE_" << m_uid << "_HW_AEC;\n";
	}
	if (m_interfaceFeedBack != null) {
		*_io << "	" << m_interfaceFeedBack->getDotNodeName() << " -> NODE_" << m_uid << "_HW_AEC;\n";
	}
	*_io << "	\n";
	
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
