/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "Node.h"
#include <airtio/debug.h>

#include <memory>

#undef __class__
#define __class__ "io::Node"

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

int32_t airtio::io::Node::rtAudioCallback(void* _outputBuffer,
                                          void* _inputBuffer,
                                          unsigned int _nBufferFrames,
                                          double _streamTime,
                                          airtaudio::streamStatus _status) {
	std::unique_lock<std::mutex> lock(m_mutex);
	std::chrono::system_clock::time_point ttime = std::chrono::system_clock::time_point();//std::chrono::system_clock::now();
	
	if (_outputBuffer != nullptr) {
		AIRTIO_VERBOSE("data Output");
		std::vector<int32_t> output;
		output.resize(_nBufferFrames*m_map.size(), 0);
		int16_t* outputTmp = nullptr;
		void* outputTmp2 = nullptr;
		size_t tmpSize = 0;
		for (auto &it : m_list) {
			if (it != nullptr) {
				AIRTIO_VERBOSE("    IO : " /* << std::distance(m_list.begin(), it)*/ << "/" << m_list.size() << " name="<< it->getName());
				tmpSize = _nBufferFrames;
				it->systemNeedOutputData(ttime, outputTmp2, tmpSize, sizeof(int16_t)*m_map.size());
				outputTmp = static_cast<int16_t*>(outputTmp2);
				//it->systemNeedOutputData(ttime, _outputBuffer, _nBufferFrames, sizeof(int16_t)*m_map.size());
				// Add data to the output tmp buffer :
				for (size_t kkk=0; kkk<output.size(); ++kkk) {
					output[kkk] += static_cast<int32_t>(outputTmp[kkk]);
					//*_outputBuffer++ = static_cast<int16_t>(outputTmp[kkk]);
				}
				break;
			}
		}
		int16_t* outputBuffer = static_cast<int16_t*>(_outputBuffer);
		for (size_t kkk=0; kkk<output.size(); ++kkk) {
			*outputBuffer++ = static_cast<int16_t>(std::min(std::max(INT16_MIN, output[kkk]), INT16_MAX));
			//*_outputBuffer++ = static_cast<int16_t>(output[kkk]);
		}
	}
	if (_inputBuffer != nullptr) {
		AIRTIO_INFO("data Input");
		int16_t* inputBuffer = static_cast<int16_t *>(_inputBuffer);
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (m_list[iii] != nullptr) {
				AIRTIO_INFO("    IO : " << iii+1 << "/" << m_list.size() << " name="<< m_list[iii]->getName());
				m_list[iii]->systemNewInputData(ttime, inputBuffer, _nBufferFrames);
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
	AIRTIO_INFO("-----------------------------------------------------------------");
	AIRTIO_INFO("--                       CREATE NODE                           --");
	AIRTIO_INFO("-----------------------------------------------------------------");
	// intanciate specific API ...
	m_adac.instanciate();
	
	if (m_streamName == "") {
		m_streamName = "default";
	}
	// set default channel property :
	m_map.push_back(airtalgo::channel_frontLeft);
	m_map.push_back(airtalgo::channel_frontRight);
	
	// search device ID :
	AIRTIO_INFO("Open :");
	AIRTIO_INFO("    m_streamName=" << m_streamName);
	AIRTIO_INFO("    m_freq=" << m_frequency);
	AIRTIO_INFO("    m_map=" << m_map);
	AIRTIO_INFO("    m_format=" << m_format);
	AIRTIO_INFO("    m_isInput=" << m_isInput);
	int32_t deviceId = 0;
	AIRTIO_INFO("Device list:");
	for (int32_t iii=0; iii<m_adac.getDeviceCount(); ++iii) {
		m_info = m_adac.getDeviceInfo(iii);
		AIRTIO_INFO("    " << iii << " name :" << m_info.name);
		if (m_info.name == m_streamName) {
			AIRTIO_INFO("        Select ...");
			deviceId = iii;
		}
	}
	// Open specific ID :
	m_info = m_adac.getDeviceInfo(deviceId);
	// display property :
	{
		AIRTIO_INFO("Device " << deviceId << " property :");
		AIRTIO_INFO("    probe=" << (m_info.probed==true?"true":"false"));
		AIRTIO_INFO("    name=" << m_info.name);
		AIRTIO_INFO("    outputChannels=" << m_info.outputChannels);
		AIRTIO_INFO("    inputChannels=" << m_info.inputChannels);
		AIRTIO_INFO("    duplexChannels=" << m_info.duplexChannels);
		AIRTIO_INFO("    isDefaultOutput=" << (m_info.isDefaultOutput==true?"true":"false"));
		AIRTIO_INFO("    isDefaultInput=" << (m_info.isDefaultInput==true?"true":"false"));
		//std::string rrate;
		std::stringstream rrate;
		for (int32_t jjj=0; jjj<m_info.sampleRates.size(); ++jjj) {
			rrate << m_info.sampleRates[jjj] << ";";
		}
		AIRTIO_INFO("    rates=" << rrate.str());
		switch(m_info.nativeFormats) {
			case airtaudio::SINT8:
				AIRTIO_INFO("    native Format: 8-bit signed integer");
				break;
			case airtaudio::SINT16:
				AIRTIO_INFO("    native Format: 16-bit signed integer");
				break;
			case airtaudio::SINT24:
				AIRTIO_INFO("    native Format: 24-bit signed integer");
				break;
			case airtaudio::SINT32:
				AIRTIO_INFO("    native Format: 32-bit signed integer");
				break;
			case airtaudio::FLOAT32:
				AIRTIO_INFO("    native Format: Normalized between plus/minus 1.0");
				break;
			case airtaudio::FLOAT64:
				AIRTIO_INFO("    native Format: Normalized between plus/minus 1.0");
				break;
			default:
				AIRTIO_INFO("    native Format: Unknow");
				break;
		}
	}
	
	// open Audio device:
	unsigned int nbChunk= 1024;
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
	AIRTIO_INFO("Open output stream nbChannels=" << params.nChannels);
	enum airtaudio::errorType err = airtaudio::errorNone;
	if (m_isInput == true) {
		err = m_adac.openStream(nullptr, &params,
		                        airtaudio::SINT16, m_frequency, &m_rtaudioFrameSize,
		                        std::bind(&airtio::io::Node::rtAudioCallback,
		                                  this,
		                                  std::placeholders::_1,
		                                  std::placeholders::_2,
		                                  std::placeholders::_3,
		                                  std::placeholders::_4,
		                                  std::placeholders::_5)
		                        );
	} else {
		err = m_adac.openStream(&params, nullptr,
		                        airtaudio::SINT16, m_frequency, &m_rtaudioFrameSize,
		                        std::bind(&airtio::io::Node::rtAudioCallback,
		                                  this,
		                                  std::placeholders::_1,
		                                  std::placeholders::_2,
		                                  std::placeholders::_3,
		                                  std::placeholders::_4,
		                                  std::placeholders::_5)
		                        );
	}
	if (err != airtaudio::errorNone) {
		AIRTIO_ERROR("Create stream : '" << m_streamName << "' mode=" << (m_isInput?"input":"output") << " can not create strem " << err);
	}
}

airtio::io::Node::~Node() {
	std::unique_lock<std::mutex> lock(m_mutex);
	AIRTIO_INFO("-----------------------------------------------------------------");
	AIRTIO_INFO("--                      DESTROY NODE                           --");
	AIRTIO_INFO("-----------------------------------------------------------------");
	AIRTIO_INFO("close input stream");
	if (m_adac.isStreamOpen() ) {
		m_adac.closeStream();
	}
};

void airtio::io::Node::start() {
	std::unique_lock<std::mutex> lock(m_mutex);
	AIRTIO_INFO("Start stream : '" << m_streamName << "' mode=" << (m_isInput?"input":"output") );
	enum airtaudio::errorType err = m_adac.startStream();
	if (err != airtaudio::errorNone) {
		AIRTIO_ERROR("Start stream : '" << m_streamName << "' mode=" << (m_isInput?"input":"output") << " can not start stream ... " << err);
	}
}

void airtio::io::Node::stop() {
	std::unique_lock<std::mutex> lock(m_mutex);
	AIRTIO_INFO("Stop stream : '" << m_streamName << "' mode=" << (m_isInput?"input":"output") );
	enum airtaudio::errorType err = m_adac.stopStream();
	if (err != airtaudio::errorNone) {
		AIRTIO_ERROR("Stop stream : '" << m_streamName << "' mode=" << (m_isInput?"input":"output") << " can not stop stream ... " << err);
	}
}

void airtio::io::Node::interfaceAdd(const std::shared_ptr<airtio::Interface>& _interface) {
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (_interface == m_list[iii]) {
				return;
			}
		}
		AIRTIO_INFO("ADD interface for stream : '" << m_streamName << "' mode=" << (m_isInput?"input":"output") );
		m_list.push_back(_interface);
	}
	if (m_list.size() == 1) {
		start();
	}
}

void airtio::io::Node::interfaceRemove(const std::shared_ptr<airtio::Interface>& _interface) {
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		for (size_t iii=0; iii< m_list.size(); ++iii) {
			if (_interface == m_list[iii]) {
				m_list.erase(m_list.begin()+iii);
				AIRTIO_INFO("RM interface for stream : '" << m_streamName << "' mode=" << (m_isInput?"input":"output") );
				break;
			}
		}
	}
	if (m_list.size() == 0) {
		stop();
	}
	return;
}

