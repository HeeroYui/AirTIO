/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifdef AUDIO_RIVER_BUILD_FILE

#include <audio/river/io/NodeFile.h>
#include <audio/river/debug.h>
#include <memory>

#undef __class__
#define __class__ "io::NodeFile"

int32_t audio::river::io::NodeFile::recordCallback(const void* _inputBuffer,
                                                   const audio::Time& _timeInput,
                                                   uint32_t _nbChunk,
                                                   const std::vector<audio::orchestra::status>& _status) {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO : Manage status ...
	RIVER_VERBOSE("data Input size request :" << _nbChunk << " [BEGIN] status=" << _status << " nbIO=" << m_list.size());
	newInput(_inputBuffer, _nbChunk, _timeInput);
	return 0;
}

int32_t audio::river::io::NodeFile::playbackCallback(void* _outputBuffer,
                                                     const audio::Time& _timeOutput,
                                                     uint32_t _nbChunk,
                                                     const std::vector<audio::orchestra::status>& _status) {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO : Manage status ...
	RIVER_VERBOSE("data Output size request :" << _nbChunk << " [BEGIN] status=" << _status << " nbIO=" << m_list.size());
	newOutput(_outputBuffer, _nbChunk, _timeOutput);
	return 0;
}



std::shared_ptr<audio::river::io::NodeFile> audio::river::io::NodeFile::create(const std::string& _name, const ejson::Object& _config) {
	return std::shared_ptr<audio::river::io::NodeFile>(new audio::river::io::NodeFile(_name, _config));
}

audio::river::io::NodeFile::NodeFile(const std::string& _name, const ejson::Object& _config) :
  Node(_name, _config) {
	audio::drain::IOFormatInterface interfaceFormat = getInterfaceFormat();
	audio::drain::IOFormatInterface hardwareFormat = getHarwareFormat();
	/**
		map-on:{ # select hardware interface and name
			interface:"alsa", # interface : "alsa", "pulse", "core", ...
			name:"default", # name of the interface
		},
		nb-chunk:1024 # number of chunk to open device (create the latency anf the frequency to call user)
	*/
	std::string typeInterface = audio::orchestra::type_undefined;
	std::string streamName = "default";
	const ejson::Object tmpObject = m_config["map-on"].toObject();
	if (tmpObject.exist() == false) {
		RIVER_WARNING("missing node : 'map-on' ==> auto map : 'auto:default'");
	} else {
		typeInterface = tmpObject.getStringValue("interface", audio::orchestra::type_undefined);
		if (typeInterface == "auto") {
			typeInterface = audio::orchestra::type_undefined;
		}
		streamName = tmpObject.getStringValue("name", "default");
	}
	int32_t nbChunk = m_config.getNumberValue("nb-chunk", 1024);
	
	// intanciate specific API ...
	m_interface.instanciate(typeInterface);
	m_interface.setName(_name);
	// TODO : Check return ...
	std::string type = m_config.getStringValue("type", "int16");
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
	int32_t deviceId = -1;
	/*
	// TODO : Remove this from here (create an extern interface ...)
	RIVER_INFO("Device list:");
	for (int32_t iii=0; iii<m_interface.getDeviceCount(); ++iii) {
		m_info = m_interface.getDeviceInfo(iii);
		RIVER_INFO("    " << iii << " name :" << m_info.name);
		m_info.display(2);
	}
	*/
	// special case for default IO:
	if (streamName == "default") {
		if (m_isInput == true) {
			deviceId = m_interface.getDefaultInputDevice();
		} else {
			deviceId = m_interface.getDefaultOutputDevice();
		}
	} else {
		for (int32_t iii=0; iii<m_interface.getDeviceCount(); ++iii) {
			m_info = m_interface.getDeviceInfo(iii);
			if (m_info.name == streamName) {
				RIVER_INFO("    Select ... id =" << iii);
				deviceId = iii;
			}
		}
	}
	// TODO : Check if the devace with the specific name exist ...
	/*
	if (deviceId == -1) {
		RIVER_ERROR("Can not find the " << streamName << " audio interface ... (use O default ...)");
		deviceId = 0;
	}
	*/
	
	// Open specific ID :
	if (deviceId == -1) {
		m_info = m_interface.getDeviceInfo(streamName);
	} else {
		m_info = m_interface.getDeviceInfo(deviceId);
	}
	// display property :
	{
		RIVER_INFO("Device " << deviceId << " - '" << streamName << "' property :");
		m_info.display();
		
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
	audio::orchestra::StreamParameters params;
	params.deviceId = deviceId;
	params.deviceName = streamName;
	params.nChannels = hardwareFormat.getMap().size();
	if (m_info.channels.size() < params.nChannels) {
		RIVER_CRITICAL("Can not open hardware device with more channel (" << params.nChannels << ") that is autorized by hardware (" << m_info.channels.size() << ").");
	}
	audio::orchestra::StreamOptions option;
	etk::from_string(option.mode, tmpObject->getStringValue("timestamp-mode", "soft"));
	
	RIVER_DEBUG("interfaceFormat=" << interfaceFormat);
	RIVER_DEBUG("hardwareFormat=" << hardwareFormat);
	
	m_rtaudioFrameSize = nbChunk;
	RIVER_INFO("Open output stream nbChannels=" << params.nChannels);
	enum audio::orchestra::error err = audio::orchestra::error_none;
	if (m_isInput == true) {
		m_process.setInputConfig(hardwareFormat);
		m_process.setOutputConfig(interfaceFormat);
		err = m_interface.openStream(nullptr, &params,
		                        hardwareFormat.getFormat(), hardwareFormat.getFrequency(), &m_rtaudioFrameSize,
		                        std::bind(&audio::river::io::NodeFile::recordCallback,
		                                    this,
		                                    std::placeholders::_1,
		                                    std::placeholders::_2,
		                                    std::placeholders::_5,
		                                    std::placeholders::_6),
		                        option
		                        );
	} else {
		m_process.setInputConfig(interfaceFormat);
		m_process.setOutputConfig(hardwareFormat);
		err = m_interface.openStream(&params, nullptr,
		                        hardwareFormat.getFormat(), hardwareFormat.getFrequency(), &m_rtaudioFrameSize,
		                        std::bind(&audio::river::io::NodeFile::playbackCallback,
		                                    this,
		                                    std::placeholders::_3,
		                                    std::placeholders::_4,
		                                    std::placeholders::_5,
		                                    std::placeholders::_6),
		                        option
		                        );
	}
	if (err != audio::orchestra::error_none) {
		RIVER_ERROR("Create stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " can not create stream " << err);
	}
	m_process.updateInterAlgo();
}

audio::river::io::NodeFile::~NodeFile() {
	std::unique_lock<std::mutex> lock(m_mutex);
	RIVER_INFO("close input stream");
	if (m_interface.isStreamOpen() ) {
		m_interface.closeStream();
	}
};

void audio::river::io::NodeFile::threadCallback() {
	etk::thread::setName("RIVER file-IO");
	// open the file
	
	while (m_alive == true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	
}

void audio::river::io::NodeFile::start() {
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_thread != nullptr) {
		RIVER_ERROR("Start stream : '" << m_name << "' mode=" << (m_isInput?"read":"write") << " ==> already started ..." );
		return;
	}
	m_alive = true;
	RIVER_INFO("Start stream : '" << m_name << "' mode=" << (m_isInput?"read":"write") );
	m_thread = std::make_shared<std::thread>(&audio::river::io::NodeFile::threadCallback2, this);
	m_time = audio::Time::now();
}

void audio::river::io::NodeFile::stop() {
	std::unique_lock<std::mutex> lock(m_mutex);
	m_alive = false;
	RIVER_INFO("Stop stream : '" << m_name << "' mode=" << (m_isInput?"read":"write") );
	// TODO : Need join ...
	m_thread->join();
	m_thread.reset();
}

#endif
