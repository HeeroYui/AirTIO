/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifdef AUDIO_RIVER_BUILD_PORTAUDIO

#include <audio/river/io/NodePortAudio.h>
#include <audio/river/debug.h>
#include <memory>
#include <audio/Time.h>
#include <audio/Duration.h>

static int portAudioStreamCallback(const void *_input,
                                   void *_output,
                                   unsigned long _frameCount,
                                   const PaStreamCallbackTimeInfo* _timeInfo,
                                   PaStreamCallbackFlags _statusFlags,
                                   void *_userData) {
	audio::river::io::NodePortAudio* myClass = reinterpret_cast<audio::river::io::NodePortAudio*>(_userData);
	int64_t sec = int64_t(_timeInfo->inputBufferAdcTime);
	int64_t nsec = (_timeInfo->inputBufferAdcTime-double(sec))*1000000000LL;
	audio::Time timeInput(sec, nsec);
	sec = int64_t(_timeInfo->outputBufferDacTime);
	nsec = (_timeInfo->outputBufferDacTime-double(sec))*1000000000LL;
	audio::Time timeOutput(sec, nsec);
	return myClass->duplexCallback(_input,
	                               timeInput,
	                               _output,
	                               timeOutput,
	                               _frameCount,
	                               _statusFlags);
}

int32_t audio::river::io::NodePortAudio::duplexCallback(const void* _inputBuffer,
                                                 const audio::Time& _timeInput,
                                                 void* _outputBuffer,
                                                 const audio::Time& _timeOutput,
                                                 uint32_t _nbChunk,
                                                 PaStreamCallbackFlags _status) {
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


std::shared_ptr<audio::river::io::NodePortAudio> audio::river::io::NodePortAudio::create(const std::string& _name, const ejson::Object& _config) {
	return std::shared_ptr<audio::river::io::NodePortAudio>(new audio::river::io::NodePortAudio(_name, _config));
}

audio::river::io::NodePortAudio::NodePortAudio(const std::string& _name, const ejson::Object& _config) :
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
	std::string streamName = "default";
	const ejson::Object tmpObject = m_config["map-on"].toObject();
	if (tmpObject.exist() == false) {
		RIVER_WARNING("missing node : 'map-on' ==> auto map : 'auto:default'");
	} else {
		std::string value = tmpObject.getStringValue("interface", "default");
		streamName = tmpObject.getStringValue("name", "default");
	}
	int32_t nbChunk = m_config.getNumberValue("nb-chunk", 1024);
	
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

audio::river::io::NodePortAudio::~NodePortAudio() {
	std::unique_lock<std::mutex> lock(m_mutex);
	RIVER_INFO("close input stream");
	PaError err = Pa_CloseStream( m_stream );
	if( err != paNoError ) {
		RIVER_ERROR("Remove stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " can not Remove stream ... " << Pa_GetErrorText(err));
	}
};

void audio::river::io::NodePortAudio::start() {
	std::unique_lock<std::mutex> lock(m_mutex);
	RIVER_INFO("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
	PaError err = Pa_StartStream(m_stream);
	if( err != paNoError ) {
		RIVER_ERROR("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " can not start stream ... " << Pa_GetErrorText(err));
	}
}

void audio::river::io::NodePortAudio::stop() {
	std::unique_lock<std::mutex> lock(m_mutex);
	RIVER_INFO("Stop stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") );
	PaError err = Pa_StopStream(m_stream);
	if( err != paNoError ) {
		RIVER_ERROR("Start stream : '" << m_name << "' mode=" << (m_isInput?"input":"output") << " can not stop stream ... " << Pa_GetErrorText(err));
	}
}
#endif

