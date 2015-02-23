/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifdef __AIRTAUDIO_INFERFACE__

#include <river/io/NodeAirTAudio.h>
#include <river/debug.h>

#include <memory>

#undef __class__
#define __class__ "io::NodeAirTAudio"

static std::string asString(const std::chrono::system_clock::time_point& tp) {
     // convert to system time:
     std::time_t t = std::chrono::system_clock::to_time_t(tp);
     // convert in human string
     std::string ts = std::ctime(&t);
     // remove \n
     ts.resize(ts.size()-1);
     return ts;
}

namespace std {
	static std::ostream& operator <<(std::ostream& _os, const std::chrono::system_clock::time_point& _obj) {
		std::chrono::microseconds us = std::chrono::duration_cast<std::chrono::microseconds>(_obj.time_since_epoch());
		_os << us.count();
		return _os;
	}
}


int32_t river::io::NodeAirTAudio::duplexCallback(const void* _inputBuffer,
                                                 const std::chrono::system_clock::time_point& _timeInput,
                                                 void* _outputBuffer,
                                                 const std::chrono::system_clock::time_point& _timeOutput,
                                                 uint32_t _nbChunk,
                                                 const std::vector<airtaudio::status>& _status) {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO : Manage status ...
	if (_inputBuffer != nullptr) {
		RIVER_VERBOSE("data Input size request :" << _nbChunk << " [BEGIN] status=" << _status << " nbIO=" << m_list.size());
		newInput(_inputBuffer, _nbChunk, _timeInput);
	}
	if (_outputBuffer != nullptr) {
		RIVER_VERBOSE("data Output size request :" << _nbChunk << " [BEGIN] status=" << _status << " nbIO=" << m_list.size());
		newOutput(_outputBuffer, _nbChunk, _timeOutput);
	}
	return 0;
}

int32_t river::io::NodeAirTAudio::recordCallback(const void* _inputBuffer,
                                                 const std::chrono::system_clock::time_point& _timeInput,
                                                 uint32_t _nbChunk,
                                                 const std::vector<airtaudio::status>& _status) {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO : Manage status ...
	RIVER_VERBOSE("data Input size request :" << _nbChunk << " [BEGIN] status=" << _status << " nbIO=" << m_list.size());
	newInput(_inputBuffer, _nbChunk, _timeInput);
	return 0;
}

int32_t river::io::NodeAirTAudio::playbackCallback(void* _outputBuffer,
                                                   const std::chrono::system_clock::time_point& _timeOutput,
                                                   uint32_t _nbChunk,
                                                   const std::vector<airtaudio::status>& _status) {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO : Manage status ...
	RIVER_VERBOSE("data Output size request :" << _nbChunk << " [BEGIN] status=" << _status << " nbIO=" << m_list.size());
	newOutput(_outputBuffer, _nbChunk, _timeOutput);
	return 0;
}



std::shared_ptr<river::io::NodeAirTAudio> river::io::NodeAirTAudio::create(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config) {
	return std::shared_ptr<river::io::NodeAirTAudio>(new river::io::NodeAirTAudio(_name, _config));
}

river::io::NodeAirTAudio::NodeAirTAudio(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config) :
  Node(_name, _config) {
	drain::IOFormatInterface interfaceFormat = getInterfaceFormat();
	drain::IOFormatInterface hardwareFormat = getHarwareFormat();
	/**
		map-on:{ # select hardware interface and name
			interface:"alsa", # interface : "alsa", "pulse", "core", ...
			name:"default", # name of the interface
		},
		nb-chunk:1024 # number of chunk to open device (create the latency anf the frequency to call user)
	*/
	enum airtaudio::type typeInterface = airtaudio::type_undefined;
	std::string streamName = "default";
	const std::shared_ptr<const ejson::Object> tmpObject = m_config->getObject("map-on");
	if (tmpObject == nullptr) {
		RIVER_WARNING("missing node : 'map-on' ==> auto map : 'auto:default'");
	} else {
		std::string value = tmpObject->getStringValue("interface", "default");
		typeInterface = airtaudio::getTypeFromString(value);
		streamName = tmpObject->getStringValue("name", "default");
	}
	int32_t nbChunk = m_config->getNumberValue("nb-chunk", 1024);
	
	// intanciate specific API ...
	m_adac.instanciate(typeInterface);
	m_adac.setName(_name);
	// TODO : Check return ...
	std::string type = m_config->getStringValue("type", "int16");
	if (streamName == "") {
		streamName = "default";
	}
	
	// search device ID :
	RIVER_INFO("Open :");
	RIVER_INFO("    m_streamName=" << streamName);
	RIVER_INFO("    m_freq=" << hardwareFormat.getFrequency());
	RIVER_INFO("    m_map=" << hardwareFormat.getMap());
	RIVER_INFO("    m_format=" << hardwareFormat.getFormat());
	RIVER_INFO("    m_isInput=" << m_isInput);
	int32_t deviceId = 0;
	RIVER_INFO("Device list:");
	for (int32_t iii=0; iii<m_adac.getDeviceCount(); ++iii) {
		m_info = m_adac.getDeviceInfo(iii);
		RIVER_INFO("    " << iii << " name :" << m_info.name);
		if (m_info.name == streamName) {
			RIVER_INFO("        Select ...");
			deviceId = iii;
		}
	}
	// Open specific ID :
	m_info = m_adac.getDeviceInfo(deviceId);
	// display property :
	{
		RIVER_INFO("Device " << deviceId << " property :");
		RIVER_INFO("    probe=" << m_info.probed);
		RIVER_INFO("    name=" << m_info.name);
		RIVER_INFO("    outputChannels=" << m_info.outputChannels);
		RIVER_INFO("    inputChannels=" << m_info.inputChannels);
		RIVER_INFO("    duplexChannels=" << m_info.duplexChannels);
		RIVER_INFO("    isDefaultOutput=" << m_info.isDefaultOutput);
		RIVER_INFO("    isDefaultInput=" << m_info.isDefaultInput);
		RIVER_INFO("    rates=" << m_info.sampleRates);
		RIVER_INFO("    native Format: " << m_info.nativeFormats);
		
		if (etk::isIn(hardwareFormat.getFormat(), m_info.nativeFormats) == false) {
			if (type == "auto") {
				if (etk::isIn(audio::format_int16, m_info.nativeFormats) == true) {
					hardwareFormat.setFormat(audio::format_int16);
					RIVER_INFO("auto set format: " << hardwareFormat.getFormat());
				} else if (etk::isIn(audio::format_float, m_info.nativeFormats) == true) {
					hardwareFormat.setFormat(audio::format_float);
					RIVER_INFO("auto set format: " << hardwareFormat.getFormat());
				} else if (etk::isIn(audio::format_int16_on_int32, m_info.nativeFormats) == true) {
					hardwareFormat.setFormat(audio::format_int16_on_int32);
					RIVER_INFO("auto set format: " << hardwareFormat.getFormat());
				} else if (etk::isIn(audio::format_int24, m_info.nativeFormats) == true) {
					hardwareFormat.setFormat(audio::format_int24);
					RIVER_INFO("auto set format: " << hardwareFormat.getFormat());
				} else if (m_info.nativeFormats.size() != 0) {
					hardwareFormat.setFormat(m_info.nativeFormats[0]);
					RIVER_INFO("auto set format: " << hardwareFormat.getFormat());
				} else {
					RIVER_CRITICAL("auto set format no element in the configuration: " << m_info.nativeFormats);
				}
			} else {
				RIVER_CRITICAL("Can not manage input transforamtion: " << hardwareFormat.getFormat() << " not in " << m_info.nativeFormats);
			}
		}
		if (etk::isIn(hardwareFormat.getFrequency(), m_info.sampleRates) == false) {
			if (etk::isIn(48000, m_info.sampleRates) == true) {
				hardwareFormat.setFrequency(48000);
				RIVER_INFO("auto set frequency: " << hardwareFormat.getFrequency());
			} else if (etk::isIn(44100, m_info.sampleRates) == true) {
				hardwareFormat.setFrequency(44100);
				RIVER_INFO("auto set frequency: " << hardwareFormat.getFrequency());
			} else if (etk::isIn(32000, m_info.sampleRates) == true) {
				hardwareFormat.setFrequency(32000);
				RIVER_INFO("auto set frequency: " << hardwareFormat.getFrequency());
			} else if (etk::isIn(16000, m_info.sampleRates) == true) {
				hardwareFormat.setFrequency(16000);
				RIVER_INFO("auto set frequency: " << hardwareFormat.getFrequency());
			} else if (etk::isIn(8000, m_info.sampleRates) == true) {
				hardwareFormat.setFrequency(8000);
				RIVER_INFO("auto set frequency: " << hardwareFormat.getFrequency());
			} else if (etk::isIn(96000, m_info.sampleRates) == true) {
				hardwareFormat.setFrequency(96000);
				RIVER_INFO("auto set frequency: " << hardwareFormat.getFrequency());
			} else if (m_info.sampleRates.size() != 0) {
				hardwareFormat.setFrequency(m_info.sampleRates[0]);
				RIVER_INFO("auto set frequency: " << hardwareFormat.getFrequency() << "(first element in list) in " << m_info.sampleRates);
			} else {
				RIVER_CRITICAL("Can not manage input transforamtion:" << hardwareFormat.getFrequency() << " not in " << m_info.sampleRates);
			}
			interfaceFormat.setFrequency(hardwareFormat.getFrequency());
		}
	}
	
	// open Audio device:
	airtaudio::StreamParameters params;
	params.deviceId = deviceId;
	if (m_isInput == true) {
		m_info.inputChannels = 2;
		params.nChannels = 2;
	} else {
		m_info.outputChannels = 2;
		params.nChannels = 2;
	}
	
	m_rtaudioFrameSize = nbChunk;
	RIVER_INFO("Open output stream nbChannels=" << params.nChannels);
	enum airtaudio::error err = airtaudio::error_none;
	if (m_isInput == true) {
		err = m_adac.openStream(nullptr, &params,
		                        hardwareFormat.getFormat(), hardwareFormat.getFrequency(), &m_rtaudioFrameSize,
		                        std::bind(&river::io::NodeAirTAudio::recordCallback,
		                                  this,
		                                  std::placeholders::_1,
		                                  std::placeholders::_2,
		                                  std::placeholders::_5,
		                                  std::placeholders::_6)
		                        );
	} else {
		err = m_adac.openStream(&params, nullptr,
		                        hardwareFormat.getFormat(), hardwareFormat.getFrequency(), &m_rtaudioFrameSize,
		                        std::bind(&river::io::NodeAirTAudio::playbackCallback,
		                                  this,
		                                  std::placeholders::_3,
		                                  std::placeholders::_4,
		                                  std::placeholders::_5,
		                                  std::placeholders::_6)
		                        );
	}
	if (err != airtaudio::error_none) {
		RIVER_ERROR("Create stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " can not create stream " << err);
	}
	m_process.updateInterAlgo();
}

river::io::NodeAirTAudio::~NodeAirTAudio() {
	std::unique_lock<std::mutex> lock(m_mutex);
	RIVER_INFO("close input stream");
	if (m_adac.isStreamOpen() ) {
		m_adac.closeStream();
	}
};

void river::io::NodeAirTAudio::start() {
	std::unique_lock<std::mutex> lock(m_mutex);
	RIVER_INFO("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
	enum airtaudio::error err = m_adac.startStream();
	if (err != airtaudio::error_none) {
		RIVER_ERROR("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " can not start stream ... " << err);
	}
}

void river::io::NodeAirTAudio::stop() {
	std::unique_lock<std::mutex> lock(m_mutex);
	RIVER_INFO("Stop stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
	enum airtaudio::error err = m_adac.stopStream();
	if (err != airtaudio::error_none) {
		RIVER_ERROR("Stop stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " can not stop stream ... " << err);
	}
}

#endif
