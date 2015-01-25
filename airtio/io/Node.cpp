/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "Node.hpp"

#include <memory>

#ifndef INT16_MAX
	#define INT16_MAX 0x7fff
#endif
#ifndef INT16_MIN
	#define INT16_MIN (-INT16_MAX - 1)
#endif
#ifndef INT32_MAX
	#define INT32_MAX 0x7fffffffL
#endif
#ifndef INT32_MIN
	#define INT32_MIN (-INT32_MAX - 1L)
#endif

// RT audio out callback
static int rtAudioCallbackStatic(void* _outputBuffer,
                                 void* _inputBuffer,
                                 unsigned int _nBufferFrames,
                                 double _streamTime,
                                 RtAudioStreamStatus _status,
                                 void* _data) {
	airtio::io::Node* interface = static_cast<airtio::io::Node*>(_data);
	return interface->rtAudioCallback(static_cast<int16_t*>(_outputBuffer),
	                                  static_cast<int16_t*>(_inputBuffer),
	                                  _nBufferFrames,
	                                  _streamTime,
	                                  _status);
}

int airtio::io::Node::rtAudioCallback(int16_t* _outputBuffer,
                                               int16_t* _inputBuffer,
                                               unsigned int _nBufferFrames,
                                               double _streamTime,
                                               RtAudioStreamStatus _status) {
	std::mutex::scoped_lock lock(m_mutex);
	
	std::chrono::system_clock::time_point ttime = boost::chrono::system_clock::time_point();//boost::chrono::system_clock::now();
	
	if (_outputBuffer != nullptr) {
		//std::cout << "data Output" << std::endl;
		std::vector<int32_t> output;
		output.resize(_nBufferFrames*m_map.size(), 0);
		std::vector<int16_t> outputTmp;
		outputTmp.resize(_nBufferFrames*m_map.size());
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (m_list[iii] != nullptr) {
				//std::cout << "    IO : " << iii+1 << "/" << m_list.size() << " name="<< m_list[iii]->getName() << std::endl;
				m_list[iii]->systemNeedOutputData(ttime, &outputTmp[0], _nBufferFrames, sizeof(int16_t)*m_map.size());
				//m_list[iii]->systemNeedOutputData(ttime, _outputBuffer, _nBufferFrames, sizeof(int16_t)*m_map.size());
				// Add data to the output tmp buffer :
				for (size_t kkk=0; kkk<output.size(); ++kkk) {
					output[kkk] += static_cast<int32_t>(outputTmp[kkk]);
					//*_outputBuffer++ = static_cast<int16_t>(outputTmp[kkk]);
				}
				break;
			}
		}
		for (size_t kkk=0; kkk<output.size(); ++kkk) {
			*_outputBuffer++ = static_cast<int16_t>(std::min(std::max(INT16_MIN, output[kkk]), INT16_MAX));
			//*_outputBuffer++ = static_cast<int16_t>(output[kkk]);
		}
	}
	if (_inputBuffer != nullptr) {
		std::cout << "data Input" << std::endl;
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (m_list[iii] != nullptr) {
				std::cout << "    IO : " << iii+1 << "/" << m_list.size() << " name="<< m_list[iii]->getName() << std::endl;
				m_list[iii]->systemNewInputData(ttime, _inputBuffer, _nBufferFrames);
			}
		}
	}
	return 0;
}


std::shared_ptr<airtio::io::Node> airtio::io::Node::create(const std::string& _streamName, bool _isInput) {
	return std::shared_ptr<airtio::io::Node>(new airtio::io::Node(_streamName, _isInput));
}

airtio::io::Node::Node(const std::string& _streamName, bool _isInput) :
  m_streamName(_streamName),
  m_frequency(48000),
  m_format(airtalgo::format_int16),
  m_isInput(_isInput) {
	std::cout << "-----------------------------------------------------------------" << std::endl;
	std::cout << "--                       CREATE NODE                           --" << std::endl;
	std::cout << "-----------------------------------------------------------------" << std::endl;
	
	
	if (m_streamName == "") {
		m_streamName = "default";
	}
	// set default channel property :
	m_map.push_back(airtalgo::channel_frontLeft);
	m_map.push_back(airtalgo::channel_frontRight);
	
	// search device ID :
	std::cout << "Open :" << std::endl;
	std::cout << "    m_streamName=" << m_streamName << std::endl;
	std::cout << "    m_freq=" << m_frequency << std::endl;
	std::cout << "    m_map=" << m_map << std::endl;
	std::cout << "    m_format=" << m_format << std::endl;
	std::cout << "    m_isInput=" << m_isInput << std::endl;
	int32_t deviceId = 0;
	std::cout << "Device list:" << std::endl;
	for (int32_t iii=0; iii<m_adac.getDeviceCount(); ++iii) {
		m_info = m_adac.getDeviceInfo(iii);
		std::cout << "    " << iii << " name :" << m_info.name << std::endl;
		if (m_info.name == m_streamName) {
			std::cout << "        Select ..." << std::endl;
			deviceId = iii;
		}
	}
	// Open specific ID :
	m_info = m_adac.getDeviceInfo(deviceId);
	// display property :
	{
		std::cout << "Device " << deviceId << " property :" << std::endl;
		std::cout << "    probe=" << (m_info.probed==true?"true":"false") << std::endl;
		std::cout << "    name=" << m_info.name << std::endl;
		std::cout << "    outputChannels=" << m_info.outputChannels << std::endl;
		std::cout << "    inputChannels=" << m_info.inputChannels << std::endl;
		std::cout << "    duplexChannels=" << m_info.duplexChannels << std::endl;
		std::cout << "    isDefaultOutput=" << (m_info.isDefaultOutput==true?"true":"false") << std::endl;
		std::cout << "    isDefaultInput=" << (m_info.isDefaultInput==true?"true":"false") << std::endl;
		//std::string rrate;
		std::stringstream rrate;
		for (int32_t jjj=0; jjj<m_info.sampleRates.size(); ++jjj) {
			rrate << m_info.sampleRates[jjj] << ";";
		}
		std::cout << "    rates=" << rrate.str() << std::endl;
		switch(m_info.nativeFormats) {
			case RTAUDIO_SINT8:
				std::cout << "    native Format: 8-bit signed integer" << std::endl;
				break;
			case RTAUDIO_SINT16:
				std::cout << "    native Format: 16-bit signed integer" << std::endl;
				break;
			case RTAUDIO_SINT24:
				std::cout << "    native Format: 24-bit signed integer" << std::endl;
				break;
			case RTAUDIO_SINT32:
				std::cout << "    native Format: 32-bit signed integer" << std::endl;
				break;
			case RTAUDIO_FLOAT32:
				std::cout << "    native Format: Normalized between plus/minus 1.0" << std::endl;
				break;
			case RTAUDIO_FLOAT64:
				std::cout << "    native Format: Normalized between plus/minus 1.0" << std::endl;
				break;
			default:
				std::cout << "    native Format: Unknow" << std::endl;
				break;
		}
	}
	
	// open Audio device:
	unsigned int nbChunk= 1024;
	RtAudio::StreamParameters params;
	params.deviceId = deviceId;
	if (m_isInput == true) {
		m_info.inputChannels = 2;
		params.nChannels = 2;
	} else {
		m_info.outputChannels = 2;
		params.nChannels = 2;
	}
	
	m_rtaudioFrameSize = nbChunk;
	std::cout << "Open output stream nbChannels=" << params.nChannels << std::endl;
	try {
		if (m_isInput == true) {
			m_adac.openStream( nullptr, &params, RTAUDIO_SINT16, m_frequency, &m_rtaudioFrameSize, &rtAudioCallbackStatic, (void *)this );
		} else {
			m_adac.openStream( &params, nullptr, RTAUDIO_SINT16, m_frequency, &m_rtaudioFrameSize, &rtAudioCallbackStatic, (void *)this );
		}
	} catch ( RtAudioError& e ) {
		e.printMessage();
		std::cout << "[ERROR]Can not open device Output" << std::endl;
		return;
	}
}

airtio::io::Node::~Node() {
	std::mutex::scoped_lock lock(m_mutex);
	std::cout << "-----------------------------------------------------------------" << std::endl;
	std::cout << "--                       DESTRO NODE                           --" << std::endl;
	std::cout << "-----------------------------------------------------------------" << std::endl;
	std::cout << "close input stream" << std::endl;
	if (m_adac.isStreamOpen() ) {
		m_adac.closeStream();
	}
};

void airtio::io::Node::start() {
	std::mutex::scoped_lock lock(m_mutex);
	std::cout << "[INFO] Start stream : '" << m_streamName << "' mode=" << (m_isInput?"input":"output")<< std::endl;
	try {
		m_adac.startStream();
	} catch (RtAudioError& e ) {
		e.printMessage();
		std::cout << "[ERROR] Can not start stream Input" << std::endl;
		if (m_adac.isStreamOpen() ) {
			m_adac.closeStream();
		}
		return;
	}
}

void airtio::io::Node::stop() {
	std::mutex::scoped_lock lock(m_mutex);
	std::cout << "[INFO] Stop stream : '" << m_streamName << "' mode=" << (m_isInput?"input":"output")<< std::endl;
	try {
		m_adac.stopStream();
	} catch ( RtAudioError& e ) {
		e.printMessage();
		std::cout << "[ERROR] Can not stop stream" << std::endl;
		if (m_adac.isStreamOpen() ) {
			m_adac.closeStream();
		}
		return;
	}
}




void airtio::io::Node::interfaceAdd(const std::shared_ptr<airtio::Interface>& _interface) {
	for (size_t iii=0; iii< m_list.size(); ++iii) {
		if (_interface == m_list[iii]) {
			return;
		}
	}
	m_list.push_back(_interface);
	start();
}

void airtio::io::Node::interfaceRemove(const std::shared_ptr<airtio::Interface>& _interface) {
	for (size_t iii=0; iii< m_list.size(); ++iii) {
		if (_interface == m_list[iii]) {
			m_list.erase(m_list.begin()+iii);
			break;
		}
	}
	if (m_list.size() == 0) {
		stop();
	}
	return;
}

