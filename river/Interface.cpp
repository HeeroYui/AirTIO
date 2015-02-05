/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "debug.h"
#include "Interface.h"
#include "io/Node.h"
#include <drain/EndPointCallback.h>
#include <drain/EndPointWrite.h>
#include <drain/EndPointRead.h>
#include <drain/Volume.h>


#undef __class__
#define __class__ "Interface"

river::Interface::Interface(void) :
  m_node(nullptr),
  m_freq(8000),
  m_map(),
  m_format(audio::format_int16),
  m_name(""),
  m_volume(0.0f) {
	
}

bool river::Interface::init(const std::string& _name,
                             float _freq,
                             const std::vector<audio::channel>& _map,
                             audio::format _format,
                             const std::shared_ptr<river::io::Node>& _node) {
	m_name = _name;
	m_node = _node;
	m_freq = _freq;
	m_map = _map;
	m_format = _format;
	m_process = std::make_shared<drain::Process>();
	m_volume = 0.0f;
	// register interface to be notify from the volume change.
	m_node->registerAsRemote(shared_from_this());
	// Create convertion interface
	if (m_node->isInput() == true) {
		// add all time the volume stage :
		std::shared_ptr<drain::Volume> algo = drain::Volume::create();
		algo->setInputFormat(m_node->getInterfaceFormat());
		algo->setName("volume");
		m_process->pushBack(algo);
		AIRTIO_INFO("add basic volume stage (1)");
		std::shared_ptr<drain::VolumeElement> tmpVolume = m_node->getVolume();
		if (tmpVolume != nullptr) {
			AIRTIO_INFO(" add volume for node");
			algo->addVolumeStage(tmpVolume);
		}
	} else {
		// add all time the volume stage :
		std::shared_ptr<drain::Volume> algo = drain::Volume::create();
		algo->setOutputFormat(m_node->getInterfaceFormat());
		algo->setName("volume");
		m_process->pushBack(algo);
		AIRTIO_INFO("add basic volume stage (2)");
		std::shared_ptr<drain::VolumeElement> tmpVolume = m_node->getVolume();
		if (tmpVolume != nullptr) {
			AIRTIO_INFO(" add volume for node");
			algo->addVolumeStage(tmpVolume);
		}
	}
	return true;
}

std::shared_ptr<river::Interface> river::Interface::create(const std::string& _name,
                                                             float _freq,
                                                             const std::vector<audio::channel>& _map,
                                                             audio::format _format,
                                                             const std::shared_ptr<river::io::Node>& _node) {
	std::shared_ptr<river::Interface> out = std::shared_ptr<river::Interface>(new river::Interface());
	out->init(_name, _freq, _map, _format, _node);
	return out;
}

river::Interface::~Interface() {
	//stop(true, true);
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	//m_node->interfaceRemove(shared_from_this());
	m_process.reset();
}
/*
bool river::Interface::hasEndPoint() {
	
}
*/
void river::Interface::setReadwrite() {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_process->removeAlgoDynamic();
	if (m_process->hasType<drain::EndPoint>() ) {
		AIRTIO_ERROR("Endpoint is already present ==> can not change");
		return;
	}
	if (m_node->isInput() == true) {
		m_process->removeIfLast<drain::EndPoint>();
		std::shared_ptr<drain::EndPointRead> algo = drain::EndPointRead::create();
		///algo->setInputFormat(drain::IOFormatInterface(m_map, m_format, m_freq));
		algo->setOutputFormat(drain::IOFormatInterface(m_map, m_format, m_freq));
		m_process->pushBack(algo);
	} else {
		m_process->removeIfFirst<drain::EndPoint>();
		std::shared_ptr<drain::EndPointWrite> algo = drain::EndPointWrite::create();
		algo->setInputFormat(drain::IOFormatInterface(m_map, m_format, m_freq));
		//algo->setOutputFormat(drain::IOFormatInterface(m_map, m_format, m_freq));
		m_process->pushFront(algo);
	}
}

void river::Interface::setOutputCallback(size_t _chunkSize, drain::needDataFunction _function) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_process->removeAlgoDynamic();
	m_process->removeIfFirst<drain::EndPoint>();
	std::shared_ptr<drain::Algo> algo = drain::EndPointCallback::create(_function);
	AIRTIO_INFO("set property: " << m_map << " " << m_format << " " << m_freq);
	algo->setInputFormat(drain::IOFormatInterface(m_map, m_format, m_freq));
	//algo->setOutputFormat(drain::IOFormatInterface(m_map, m_format, m_freq));
	m_process->pushFront(algo);
}

void river::Interface::setInputCallback(size_t _chunkSize, drain::haveNewDataFunction _function) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_process->removeAlgoDynamic();
	m_process->removeIfLast<drain::EndPoint>();
	std::shared_ptr<drain::Algo> algo = drain::EndPointCallback::create(_function);
	//algo->setInputFormat(drain::IOFormatInterface(m_map, m_format, m_freq));
	algo->setOutputFormat(drain::IOFormatInterface(m_map, m_format, m_freq));
	m_process->pushBack(algo);
}

void river::Interface::setWriteCallback(drain::needDataFunctionWrite _function) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_process->removeAlgoDynamic();
	std::shared_ptr<drain::EndPointWrite> algo = m_process->get<drain::EndPointWrite>(0);
	if (algo == nullptr) {
		return;
	}
	algo->setCallback(_function);
}

void river::Interface::start(const std::chrono::system_clock::time_point& _time) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	AIRTIO_DEBUG("start [BEGIN]");
	m_process->updateInterAlgo();
	m_node->interfaceAdd(shared_from_this());
	AIRTIO_DEBUG("start [ END ]");
}

void river::Interface::stop(bool _fast, bool _abort) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	AIRTIO_DEBUG("stop [BEGIN]");
	m_node->interfaceRemove(shared_from_this());
	AIRTIO_DEBUG("stop [ END]");
}

void river::Interface::abort() {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	AIRTIO_DEBUG("abort [BEGIN]");
	// TODO :...
	AIRTIO_DEBUG("abort [ END ]");
}

bool river::Interface::setParameter(const std::string& _filter, const std::string& _parameter, const std::string& _value) {
	AIRTIO_DEBUG("setParameter [BEGIN] : '" << _filter << "':'" << _parameter << "':'" << _value << "'");
	bool out = false;
	if (    _filter == "volume"
	     && _parameter != "FLOW") {
		AIRTIO_ERROR("Interface is not allowed to modify '" << _parameter << "' Volume just allowed to modify 'FLOW' volume");
		return false;
	}
	std::shared_ptr<drain::Algo> algo = m_process->get<drain::Algo>(_filter);
	if (algo == nullptr) {
		AIRTIO_ERROR("setParameter(" << _filter << ") ==> no filter named like this ...");
		return false;
	}
	out = algo->setParameter(_parameter, _value);
	AIRTIO_DEBUG("setParameter [ END ] : '" << out << "'");
	return out;
}
std::string river::Interface::getParameter(const std::string& _filter, const std::string& _parameter) const {
	AIRTIO_DEBUG("getParameter [BEGIN] : '" << _filter << "':'" << _parameter << "'");
	std::string out;
	std::shared_ptr<drain::Algo> algo = m_process->get<drain::Algo>(_filter);
	if (algo == nullptr) {
		AIRTIO_ERROR("setParameter(" << _filter << ") ==> no filter named like this ...");
		return "[ERROR]";
	}
	out = algo->getParameter(_parameter);
	AIRTIO_DEBUG("getParameter [ END ] : '" << out << "'");
	return out;
}
std::string river::Interface::getParameterProperty(const std::string& _filter, const std::string& _parameter) const {
	AIRTIO_DEBUG("getParameterProperty [BEGIN] : '" << _filter << "':'" << _parameter << "'");
	std::string out;
	std::shared_ptr<drain::Algo> algo = m_process->get<drain::Algo>(_filter);
	if (algo == nullptr) {
		AIRTIO_ERROR("setParameter(" << _filter << ") ==> no filter named like this ...");
		return "[ERROR]";
	}
	out = algo->getParameterProperty(_parameter);
	AIRTIO_DEBUG("getParameterProperty [ END ] : '" << out << "'");
	return out;
}

void river::Interface::write(const void* _value, size_t _nbChunk) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_process->updateInterAlgo();
	std::shared_ptr<drain::EndPointWrite> algo = m_process->get<drain::EndPointWrite>(0);
	if (algo == nullptr) {
		return;
	}
	algo->write(_value, _nbChunk);
}

#if 0
// TODO : add API aCCess mutex for Read and write...
std::vector<int16_t> river::Interface::read(size_t _nbChunk) {
	// TODO :...
	std::vector<int16_t> data;
	/*
	data.resize(_nbChunk*m_map.size(), 0);
	m_mutex.lock();
	int32_t nbChunkBuffer = m_circularBuffer.size() / m_map.size();
	m_mutex.unlock();
	while (nbChunkBuffer < _nbChunk) {
		usleep(1000);
		nbChunkBuffer = m_circularBuffer.size() / m_map.size();
	}
	m_mutex.lock();
	for (size_t iii = 0; iii<data.size(); ++iii) {
		data[iii] = m_circularBuffer[iii];
	}
	m_circularBuffer.erase(m_circularBuffer.begin(), m_circularBuffer.begin()+data.size());
	m_mutex.unlock();
	*/
	return data;
}
#endif

void river::Interface::read(void* _value, size_t _nbChunk) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_process->updateInterAlgo();
	// TODO :...
	
}

size_t river::Interface::size() const {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	// TODO :...
	return 0;
}

void river::Interface::setBufferSize(size_t _nbChunk) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_process->updateInterAlgo();
	// TODO :...
	
}

void river::Interface::setBufferSize(const std::chrono::duration<int64_t, std::micro>& _time) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_process->updateInterAlgo();
	// TODO :...
	
}

void river::Interface::clearInternalBuffer() {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	m_process->updateInterAlgo();
	// TODO :...
	
}

std::chrono::system_clock::time_point river::Interface::getCurrentTime() const {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	// TODO :...
	return std::chrono::system_clock::time_point();
	return std::chrono::system_clock::now();
}

void river::Interface::addVolumeGroup(const std::string& _name) {
	std::unique_lock<std::recursive_mutex> lock(m_mutex);
	AIRTIO_DEBUG("addVolumeGroup(" << _name << ")");
	std::shared_ptr<drain::Volume> algo = m_process->get<drain::Volume>("volume");
	if (algo == nullptr) {
		AIRTIO_ERROR("addVolumeGroup(" << _name << ") ==> no volume stage ... can not add it ...");
		return;
	}
	if (_name == "FLOW") {
		// Local volume name
		algo->addVolumeStage(std::make_shared<drain::VolumeElement>(_name));
	} else {
		// get manager unique instance:
		std::shared_ptr<river::io::Manager> mng = river::io::Manager::getInstance();
		algo->addVolumeStage(mng->getVolumeGroup(_name));
	}
}

void river::Interface::systemNewInputData(std::chrono::system_clock::time_point _time, void* _data, size_t _nbChunk) {
	std::unique_lock<std::recursive_mutex> lockProcess(m_mutex);
	m_process->push(_time, _data, _nbChunk);
}

void river::Interface::systemNeedOutputData(std::chrono::system_clock::time_point _time, void* _data, size_t _nbChunk, size_t _chunkSize) {
	std::unique_lock<std::recursive_mutex> lockProcess(m_mutex);
	m_process->pull(_time, _data, _nbChunk, _chunkSize);
}

void river::Interface::systemVolumeChange() {
	std::unique_lock<std::recursive_mutex> lockProcess(m_mutex);
	std::shared_ptr<drain::Volume> algo = m_process->get<drain::Volume>("volume");
	if (algo == nullptr) {
		return;
	}
	algo->volumeChange();
}