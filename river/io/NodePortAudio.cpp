/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifdef __PORTAUDIO_INFERFACE__

#include <river/io/NodePortAudio.h>
#include <river/debug.h>

#if __cplusplus >= 201103L
	#include <memory>
#else
	#include <etk/memory.h>
#endif

#undef __class__
#define __class__ "io::NodePortAudio"

static std::string asString(const std11::chrono::system_clock::time_point& tp) {
     // convert to system time:
     std::time_t t = std11::chrono::system_clock::to_time_t(tp);
     // convert in human string
     std::string ts = std::ctime(&t);
     // remove \n
     ts.resize(ts.size()-1);
     return ts;
}

namespace std {
	static std::ostream& operator <<(std::ostream& _os, const std11::chrono::system_clock::time_point& _obj) {
		std11::chrono::microseconds us = std11::chrono::duration_cast<std11::chrono::microseconds>(_obj.time_since_epoch());
		_os << us.count();
		return _os;
	}
}

static int portAudioStreamCallback(const void *_input,
                                   void *_output,
                                   unsigned long _frameCount,
                                   const PaStreamCallbackTimeInfo* _timeInfo,
                                   PaStreamCallbackFlags _statusFlags,
                                   void *_userData) {
	river::io::NodePortAudio* myClass = reinterpret_cast<river::io::NodePortAudio*>(_userData);
	int64_t sec = int64_t(_timeInfo->inputBufferAdcTime);
	int64_t nsec = (_timeInfo->inputBufferAdcTime-double(sec))*1000000000LL;
	std11::chrono::system_clock::time_point timeInput = std11::chrono::system_clock::from_time_t(sec) + std11::chrono::nanoseconds(nsec);
	sec = int64_t(_timeInfo->outputBufferDacTime);
	nsec = (_timeInfo->outputBufferDacTime-double(sec))*1000000000LL;
	std11::chrono::system_clock::time_point timeOutput = std11::chrono::system_clock::from_time_t(sec) + std11::chrono::nanoseconds(nsec);
	return myClass->duplexCallback(_input,
	                               timeInput,
	                               _output,
	                               timeOutput,
	                               _frameCount,
	                               _statusFlags);
}

int32_t river::io::NodePortAudio::duplexCallback(const void* _inputBuffer,
                                                 const std11::chrono::system_clock::time_point& _timeInput,
                                                 void* _outputBuffer,
                                                 const std11::chrono::system_clock::time_point& _timeOutput,
                                                 uint32_t _nbChunk,
                                                 PaStreamCallbackFlags _status) {
	std11::unique_lock<std11::mutex> lock(m_mutex);
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


std11::shared_ptr<river::io::NodePortAudio> river::io::NodePortAudio::create(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config) {
	return std11::shared_ptr<river::io::NodePortAudio>(new river::io::NodePortAudio(_name, _config));
}

river::io::NodePortAudio::NodePortAudio(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config) :
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
	const std11::shared_ptr<const ejson::Object> tmpObject = m_config->getObject("map-on");
	if (tmpObject == nullptr) {
		RIVER_WARNING("missing node : 'map-on' ==> auto map : 'auto:default'");
	} else {
		std::string value = tmpObject->getStringValue("interface", "default");
		typeInterface = airtaudio::getTypeFromString(value);
		streamName = tmpObject->getStringValue("name", "default");
	}
	int32_t nbChunk = m_config->getNumberValue("nb-chunk", 1024);
	
	PaError err = 0;
	if (m_isInput == true) {
		err = Pa_OpenDefaultStream(&m_stream,
		                           hardwareFormat.getMap().size(),
		                           0,
		                           paInt16,
		                           hardwareFormat.getFrequency(),
		                           nbChunk,
		                           &portAudioStreamCallback,
		                           this);
	} else {
		err = Pa_OpenDefaultStream(&m_stream,
		                           0,
		                           hardwareFormat.getMap().size(),
		                           paInt16,
		                           hardwareFormat.getFrequency(),
		                           nbChunk,
		                           &portAudioStreamCallback,
		                           this);
	}
	if( err != paNoError ) {
		RIVER_ERROR("Can not create Stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " err : " << Pa_GetErrorText(err));
	}
	m_process.updateInterAlgo();
}

river::io::NodePortAudio::~NodePortAudio() {
	std11::unique_lock<std11::mutex> lock(m_mutex);
	RIVER_INFO("close input stream");
	PaError err = Pa_CloseStream( m_stream );
	if( err != paNoError ) {
		RIVER_ERROR("Remove stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " can not Remove stream ... " << Pa_GetErrorText(err));
	}
};

void river::io::NodePortAudio::start() {
	std11::unique_lock<std11::mutex> lock(m_mutex);
	RIVER_INFO("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
	PaError err = Pa_StartStream(m_stream);
	if( err != paNoError ) {
		RIVER_ERROR("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " can not start stream ... " << Pa_GetErrorText(err));
	}
}

void river::io::NodePortAudio::stop() {
	std11::unique_lock<std11::mutex> lock(m_mutex);
	RIVER_INFO("Stop stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
	PaError err = Pa_StopStream(m_stream);
	if( err != paNoError ) {
		RIVER_ERROR("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " can not stop stream ... " << Pa_GetErrorText(err));
	}
}
#endif

