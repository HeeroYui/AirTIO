/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <river/io/NodeAEC.h>
#include <river/debug.h>

#include <memory>

#undef __class__
#define __class__ "io::NodeAEC"

#if 0
int32_t river::io::NodeAEC::airtAudioCallback(void* _outputBuffer,
                                                    void* _inputBuffer,
                                                    uint32_t _nbChunk,
                                                    const std::chrono::system_clock::time_point& _time,
                                                    airtaudio::status _status) {
	std::unique_lock<std::mutex> lock(m_mutex);
	//RIVER_INFO("Time=" << _time);
	/*
	for (int32_t iii=0; iii<400; ++iii) {
		RIVER_VERBOSE("dummy=" << uint64_t(dummy[iii]));
	}
	*/
	if (_outputBuffer != nullptr) {
		RIVER_VERBOSE("data Output size request :" << _nbChunk << " [BEGIN] status=" << _status << " nbIO=" << m_list.size());
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
	}
	if (_inputBuffer != nullptr) {
		RIVER_VERBOSE("data Input size request :" << _nbChunk << " [BEGIN] status=" << _status << " nbIO=" << m_list.size());
		int16_t* inputBuffer = static_cast<int16_t *>(_inputBuffer);
		for (auto &it : m_list) {
			if (it == nullptr) {
				continue;
			}
			if (it->getMode() != river::modeInterface_input) {
				continue;
			}
			RIVER_INFO("    IO name="<< it->getName());
			it->systemNewInputData(_time, inputBuffer, _nbChunk);
		}
		RIVER_VERBOSE("data Input size request :" << _nbChunk << " [ END ]");
	}
	return 0;
}
#endif

std::shared_ptr<river::io::NodeAEC> river::io::NodeAEC::create(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config) {
	return std::shared_ptr<river::io::NodeAEC>(new river::io::NodeAEC(_name, _config));
}

std::shared_ptr<river::Interface> river::io::NodeAEC::createInput(float _freq,
                                                                  const std::vector<audio::channel>& _map,
                                                                  audio::format _format,
                                                                  const std::string& _objectName,
                                                                  const std::string& _name) {
	// check if the output exist
	const std::shared_ptr<const ejson::Object> tmppp = m_config->getObject(_objectName);
	if (tmppp == nullptr) {
		RIVER_ERROR("can not open a non existance virtual interface: '" << _objectName << "' not present in : " << m_config->getKeys());
		return nullptr;
	}
	std::string streamName = tmppp->getStringValue("map-on", "error");
	
	
	// check if it is an Output:
	std::string type = tmppp->getStringValue("io", "error");
	if (    type != "input"
	     && type != "feedback") {
		RIVER_ERROR("can not open in output a virtual interface: '" << streamName << "' configured has : " << type);
		return nullptr;
	}
	// get global hardware interface:
	std::shared_ptr<river::io::Manager> manager = river::io::Manager::getInstance();
	// get the output or input channel :
	std::shared_ptr<river::io::Node> node = manager->getNode(streamName);
	// create user iterface:
	std::shared_ptr<river::Interface> interface;
	interface = river::Interface::create(_name, _freq, _map, _format, node, tmppp);
	return interface;
}


river::io::NodeAEC::NodeAEC(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config) :
  Node(_name, _config) {
	drain::IOFormatInterface interfaceFormat = getInterfaceFormat();
	drain::IOFormatInterface hardwareFormat = getHarwareFormat();
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
	std::vector<audio::channel> feedbackMap;
	feedbackMap.push_back(audio::channel_frontCenter);
	RIVER_INFO("Create FEEDBACK : ");
	m_interfaceFeedBack = createInput(hardwareFormat.getFrequency(),
	                                  feedbackMap,
	                                  hardwareFormat.getFormat(),
	                                  "map-on-feedback",
	                                  _name + "-AEC-feedback");
	if (m_interfaceFeedBack == nullptr) {
		RIVER_ERROR("Can not opne virtual device ... map-on-feedback in " << _name);
		return;
	}
	RIVER_INFO("Create MICROPHONE : ");
	m_interfaceMicrophone = createInput(hardwareFormat.getFrequency(),
	                                    hardwareFormat.getMap(),
	                                    hardwareFormat.getFormat(),
	                                    "map-on-microphone",
	                                    _name + "-AEC-microphone");
	if (m_interfaceMicrophone == nullptr) {
		RIVER_ERROR("Can not opne virtual device ... map-on-microphone in " << _name);
		return;
	}
	
	// set callback mode ...
	m_interfaceFeedBack->setInputCallback(std::bind(&river::io::NodeAEC::onDataReceivedFeedBack,
	                                                this,
	                                                std::placeholders::_1,
	                                                std::placeholders::_2,
	                                                std::placeholders::_3,
	                                                std::placeholders::_4,
	                                                std::placeholders::_5,
	                                                std::placeholders::_6));
	// set callback mode ...
	m_interfaceMicrophone->setInputCallback(std::bind(&river::io::NodeAEC::onDataReceivedMicrophone,
	                                                  this,
	                                                  std::placeholders::_1,
	                                                  std::placeholders::_2,
	                                                  std::placeholders::_3,
	                                                  std::placeholders::_4,
	                                                  std::placeholders::_5,
	                                                  std::placeholders::_6));
	
	m_bufferMicrophone.setCapacity(std::chrono::milliseconds(1000),
	                               audio::getFormatBytes(hardwareFormat.getFormat())*hardwareFormat.getMap().size(),
	                               hardwareFormat.getFrequency());
	m_bufferFeedBack.setCapacity(std::chrono::milliseconds(1000),
	                             audio::getFormatBytes(hardwareFormat.getFormat()), // only one channel ...
	                             hardwareFormat.getFrequency());
	
	m_process.updateInterAlgo();
}

river::io::NodeAEC::~NodeAEC() {
	RIVER_INFO("close input stream");
	stop();
	m_interfaceFeedBack.reset();
	m_interfaceMicrophone.reset();
};

void river::io::NodeAEC::start() {
	std::unique_lock<std::mutex> lock(m_mutex);
	RIVER_INFO("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
	if (m_interfaceFeedBack != nullptr) {
		RIVER_INFO("Start FEEDBACK : ");
		m_interfaceFeedBack->start();
	}
	if (m_interfaceMicrophone != nullptr) {
		RIVER_INFO("Start Microphone : ");
		m_interfaceMicrophone->start();
	}
}

void river::io::NodeAEC::stop() {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_interfaceFeedBack != nullptr) {
		m_interfaceFeedBack->stop();
	}
	if (m_interfaceMicrophone != nullptr) {
		m_interfaceMicrophone->stop();
	}
}

namespace std {
	static std::ostream& operator <<(std::ostream& _os, const std::chrono::system_clock::time_point& _obj) {
		std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(_obj.time_since_epoch());
		int64_t totalSecond = ns.count()/1000000000;
		int64_t millisecond = (ns.count()%1000000000)/1000000;
		int64_t microsecond = (ns.count()%1000000)/1000;
		int64_t nanosecond = ns.count()%1000;
		//_os << totalSecond << "s " << millisecond << "ms " << microsecond << "µs " << nanosecond << "ns";
		
		int32_t second = totalSecond % 60;
		int32_t minute = (totalSecond/60)%60;
		int32_t hour = (totalSecond/3600)%24;
		int32_t day = (totalSecond/(24*3600))%365;
		int32_t year = totalSecond/(24*3600*365);
		_os << year << "y " << day << "d " << hour << "h" << minute << ":"<< second << "s " << millisecond << "ms " << microsecond << "µs " << nanosecond << "ns";
		return _os;
	}
}
#define SAVE_FILE_MACRO(type,fileName,dataPointer,nbElement) \
	do { \
		static FILE *pointerOnFile = nullptr; \
		static bool errorOpen = false; \
		if (NULL==pointerOnFile) { \
			RIVER_WARNING("open file '" << fileName << "' type=" << #type); \
			pointerOnFile = fopen(fileName,"w"); \
			if (    errorOpen == false \
			     && pointerOnFile == nullptr) { \
				RIVER_ERROR("ERROR OPEN file ... '" << fileName << "' type=" << #type); \
				errorOpen=true; \
			} \
		} \
		if (pointerOnFile != nullptr) { \
			fwrite((dataPointer), sizeof(type), (nbElement), pointerOnFile); \
			fflush(pointerOnFile); \
		} \
	}while(0)


void river::io::NodeAEC::onDataReceivedMicrophone(const void* _data,
                                                  const std::chrono::system_clock::time_point& _time,
                                                  size_t _nbChunk,
                                                  enum audio::format _format,
                                                  uint32_t _frequency,
                                                  const std::vector<audio::channel>& _map) {
	RIVER_DEBUG("Microphone Time=" << _time << " _nbChunk=" << _nbChunk << " _map=" << _map << " _format=" << _format << " freq=" << _frequency);
	if (_format != audio::format_int16) {
		RIVER_ERROR("call wrong type ... (need int16_t)");
	}
	// push data synchronize
	std::unique_lock<std::mutex> lock(m_mutex);
	m_bufferMicrophone.write(_data, _nbChunk, _time);
	SAVE_FILE_MACRO(int16_t, "REC_Microphone.raw", _data, _nbChunk*_map.size());
	process();
}

void river::io::NodeAEC::onDataReceivedFeedBack(const void* _data,
                                                const std::chrono::system_clock::time_point& _time,
                                                size_t _nbChunk,
                                                enum audio::format _format,
                                                uint32_t _frequency,
                                                const std::vector<audio::channel>& _map) {
	RIVER_DEBUG("FeedBack   Time=" << _time << " _nbChunk=" << _nbChunk << " _map=" << _map << " _format=" << _format << " freq=" << _frequency);
	if (_format != audio::format_int16) {
		RIVER_ERROR("call wrong type ... (need int16_t)");
	}
	// push data synchronize
	std::unique_lock<std::mutex> lock(m_mutex);
	m_bufferFeedBack.write(_data, _nbChunk, _time);
	SAVE_FILE_MACRO(int16_t, "REC_FeedBack.raw", _data, _nbChunk*_map.size());
	process();
}

void river::io::NodeAEC::process() {
	if (m_bufferMicrophone.getSize() <= 256) {
		return;
	}
	if (m_bufferFeedBack.getSize() <= 256) {
		return;
	}
	std::chrono::system_clock::time_point MicTime = m_bufferMicrophone.getReadTimeStamp();
	std::chrono::system_clock::time_point fbTime = m_bufferFeedBack.getReadTimeStamp();
	
	// Synchronize if possible
	if (MicTime < fbTime) {
		RIVER_INFO("micTime < fbTime : Change Microphone time start " << fbTime);
		RIVER_INFO("                                 old time stamp=" << m_bufferMicrophone.getReadTimeStamp());
		m_bufferMicrophone.setReadPosition(fbTime);
		RIVER_INFO("                                 new time stamp=" << m_bufferMicrophone.getReadTimeStamp());
	}
	/*
	if (MicTime > fbTime) {
		RIVER_INFO("micTime > fbTime : Change FeedBack time start " << fbTime);
		RIVER_INFO("                               old time stamp=" << m_bufferFeedBack.getReadTimeStamp());
		m_bufferFeedBack.setReadPosition(MicTime);
		RIVER_INFO("                               new time stamp=" << m_bufferFeedBack.getReadTimeStamp());
	}*/
	// check if enought time after synchronisation ...
	if (m_bufferMicrophone.getSize() <= 256) {
		return;
	}
	if (m_bufferFeedBack.getSize() <= 256) {
		return;
	}
	
	MicTime = m_bufferMicrophone.getReadTimeStamp();
	fbTime = m_bufferFeedBack.getReadTimeStamp();
	
	if (MicTime != fbTime) {
		RIVER_ERROR("Can not synchronize flow ... : " << MicTime << " != " << fbTime << "  delta = " << (MicTime-fbTime).count()/1000 << " µs");
		return;
	}
	std::vector<uint8_t> dataMic;
	std::vector<uint8_t> dataFB;
	dataMic.resize(256*sizeof(int16_t)*2, 0);
	dataFB.resize(256*sizeof(int16_t), 0);
	while (true) {
		MicTime = m_bufferMicrophone.getReadTimeStamp();
		fbTime = m_bufferFeedBack.getReadTimeStamp();
		RIVER_INFO(" process 256 samples ... " << MicTime);
		m_bufferMicrophone.read(&dataMic[0], 256);
		m_bufferFeedBack.read(&dataFB[0], 256);
		SAVE_FILE_MACRO(int16_t, "REC_Microphone_sync.raw", &dataMic[0], 256*2);
		SAVE_FILE_MACRO(int16_t, "REC_FeedBack_sync.raw", &dataFB[0], 256);
		// if threaded : send event / otherwise, process ...
		//processAEC(&dataMic[0], &dataFB[0], 256, _time);
		if (m_bufferMicrophone.getSize() <= 256) {
			return;
		}
		if (m_bufferFeedBack.getSize() <= 256) {
			return;
		}
	}
}


void river::io::NodeAEC::processAEC(void* _dataMic, void* _dataFB, uint32_t _nbChunk, const std::chrono::system_clock::time_point& _time) {
	newInput(_dataMic, _nbChunk, _time);
}

