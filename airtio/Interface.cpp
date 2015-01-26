/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "debug.h"
#include "Interface.h"
#include "io/Node.h"
#include <airtalgo/ChannelReorder.h>
#include <airtalgo/FormatUpdate.h>
#include <airtalgo/Resampler.h>
#include <airtalgo/EndPointCallback.h>
#include <airtalgo/EndPointWrite.h>
#include <airtalgo/EndPointRead.h>


airtio::Interface::Interface(void) :
  m_node(nullptr),
  m_freq(8000),
  m_map(),
  m_format(airtalgo::format_int16),
  m_name(""),
  m_volume(0.0f) {
	
}

bool airtio::Interface::init(const std::string& _name,
                             float _freq,
                             const std::vector<airtalgo::channel>& _map,
                             airtalgo::format _format,
                             const std::shared_ptr<airtio::io::Node>& _node) {
	m_name = _name;
	m_node = _node;
	m_freq = _freq;
	m_map = _map;
	m_format = _format;
	m_process = std::make_shared<airtalgo::Process>();
	m_volume = 0.0f;
	
	// Create convertion interface
	if (m_node->isInput() == true) {
		if (m_map != m_node->getMap()) {
			std::shared_ptr<airtalgo::ChannelReorder> algo = std::make_shared<airtalgo::ChannelReorder>();
			algo->setInputFormat(airtalgo::IOFormatInterface(m_node->getMap(), m_node->getFormat(), m_node->getFrequency()));
			algo->setOutputFormat(airtalgo::IOFormatInterface(m_map, m_node->getFormat(), m_node->getFrequency()));
			m_process->pushBack(algo);
			AIRTIO_INFO("convert " << m_node->getMap() << " -> " << m_map);
		}
		if (m_freq != m_node->getFrequency()) {
			std::shared_ptr<airtalgo::Resampler> algo = std::make_shared<airtalgo::Resampler>();
			algo->setInputFormat(airtalgo::IOFormatInterface(m_map, m_node->getFormat(), m_node->getFrequency()));
			algo->setOutputFormat(airtalgo::IOFormatInterface(m_map, m_node->getFormat(), m_freq));
			m_process->pushBack(algo);
			AIRTIO_INFO("convert " << m_node->getFrequency() << " -> " << m_freq);
		}
		if (m_format != m_node->getFormat()) {
			std::shared_ptr<airtalgo::FormatUpdate> algo = std::make_shared<airtalgo::FormatUpdate>();
			algo->setInputFormat(airtalgo::IOFormatInterface(m_map, m_node->getFormat(), m_freq));
			algo->setOutputFormat(airtalgo::IOFormatInterface(m_map, m_format, m_freq));
			m_process->pushBack(algo);
			AIRTIO_INFO("convert " << m_node->getFormat() << " -> " << m_format);
		}
		// by default we add a read node
		if (true) {
			std::shared_ptr<airtalgo::EndPointRead> algo = std::make_shared<airtalgo::EndPointRead>();
			algo->setInputFormat(airtalgo::IOFormatInterface(m_map, m_format, m_freq));
			algo->setOutputFormat(airtalgo::IOFormatInterface(m_map, m_format, m_freq));
			m_process->pushBack(algo);
			AIRTIO_INFO("add default read node ...");
		}
	} else {
		// by default we add a write node:
		if (true) {
			std::shared_ptr<airtalgo::EndPointWrite> algo = std::make_shared<airtalgo::EndPointWrite>();
			algo->setInputFormat(airtalgo::IOFormatInterface(m_map, m_format, m_freq));
			algo->setOutputFormat(airtalgo::IOFormatInterface(m_map, m_format, m_freq));
			m_process->pushBack(algo);
			AIRTIO_INFO("add default write node ...");
		}
		if (m_format != m_node->getFormat()) {
			std::shared_ptr<airtalgo::FormatUpdate> algo = std::make_shared<airtalgo::FormatUpdate>();
			algo->setInputFormat(airtalgo::IOFormatInterface(m_map, m_format, m_freq));
			algo->setOutputFormat(airtalgo::IOFormatInterface(m_map, m_node->getFormat(), m_freq));
			m_process->pushBack(algo);
			AIRTIO_INFO("convert " << m_format << " -> " << m_node->getFormat());
		}
		if (m_freq != m_node->getFrequency()) {
			std::shared_ptr<airtalgo::Resampler> algo = std::make_shared<airtalgo::Resampler>();
			algo->setInputFormat(airtalgo::IOFormatInterface(m_map, m_node->getFormat(), m_freq));
			algo->setOutputFormat(airtalgo::IOFormatInterface(m_map, m_node->getFormat(), m_node->getFrequency()));
			m_process->pushBack(algo);
			AIRTIO_INFO("convert " << m_freq << " -> " << m_node->getFrequency());
		}
		if (m_map != m_node->getMap()) {
			std::shared_ptr<airtalgo::ChannelReorder> algo = std::make_shared<airtalgo::ChannelReorder>();
			algo->setInputFormat(airtalgo::IOFormatInterface(m_map, m_node->getFormat(), m_node->getFrequency()));
			algo->setOutputFormat(airtalgo::IOFormatInterface(m_node->getMap(), m_node->getFormat(), m_node->getFrequency()));
			m_process->pushBack(algo);
			AIRTIO_INFO("convert " << m_map << " -> " << m_node->getMap());
		}
	}
	//m_node->interfaceAdd(shared_from_this());
	return true;
}

std::shared_ptr<airtio::Interface> airtio::Interface::create(const std::string& _name,
                                                             float _freq,
                                                             const std::vector<airtalgo::channel>& _map,
                                                             airtalgo::format _format,
                                                             const std::shared_ptr<airtio::io::Node>& _node) {
	std::shared_ptr<airtio::Interface> out = std::shared_ptr<airtio::Interface>(new airtio::Interface());
	out->init(_name, _freq, _map, _format, _node);
	return out;
}

airtio::Interface::~Interface() {
	std::unique_lock<std::mutex> lock(m_mutex);
	//m_node->interfaceRemove(shared_from_this());
}


void airtio::Interface::setOutputCallback(size_t _chunkSize, airtalgo::needDataFunction _function, enum airtalgo::formatDataType _dataType) {
	std::unique_lock<std::mutex> lock(m_mutex);
	m_process->removeIfFirst<airtalgo::EndPoint>();
	std::shared_ptr<airtalgo::Algo> algo = std::make_shared<airtalgo::EndPointCallback>(_function, _dataType);
	AIRTIO_INFO("set property: " << m_map << " " << m_format << " " << m_freq);
	algo->setInputFormat(airtalgo::IOFormatInterface(m_map, m_format, m_freq));
	algo->setOutputFormat(airtalgo::IOFormatInterface(m_map, m_format, m_freq));
	m_process->pushFront(algo);
}

void airtio::Interface::setInputCallback(size_t _chunkSize, airtalgo::haveNewDataFunction _function, enum airtalgo::formatDataType _dataType) {
	std::unique_lock<std::mutex> lock(m_mutex);
	m_process->removeIfLast<airtalgo::EndPoint>();
	std::shared_ptr<airtalgo::Algo> algo = std::make_shared<airtalgo::EndPointCallback>(_function, _dataType);
	algo->setInputFormat(airtalgo::IOFormatInterface(m_map, m_format, m_freq));
	algo->setOutputFormat(airtalgo::IOFormatInterface(m_map, m_format, m_freq));
	m_process->pushBack(algo);
}


void airtio::Interface::start(const std::chrono::system_clock::time_point& _time) {
	std::unique_lock<std::mutex> lock(m_mutex);
	AIRTIO_DEBUG("start [BEGIN]");
	m_node->interfaceAdd(shared_from_this());
	AIRTIO_DEBUG("start [ END ]");
}

void airtio::Interface::stop(bool _fast, bool _abort) {
	std::unique_lock<std::mutex> lock(m_mutex);
	AIRTIO_DEBUG("stop [BEGIN]");
	m_node->interfaceRemove(shared_from_this());
	AIRTIO_DEBUG("stop [ END]");
}

void airtio::Interface::abort() {
	std::unique_lock<std::mutex> lock(m_mutex);
	AIRTIO_DEBUG("abort [BEGIN]");
	// TODO :...
	AIRTIO_DEBUG("abort [ END ]");
}

void airtio::Interface::setVolume(float _gainDB) {
	std::unique_lock<std::mutex> lock(m_mutex);
	AIRTIO_DEBUG("setVolume [BEGIN]");
	// TODO :...
	AIRTIO_DEBUG("setVolume [ END ]");
}

float airtio::Interface::getVolume() const {
	std::unique_lock<std::mutex> lock(m_mutex);
	AIRTIO_DEBUG("getVolume [BEGIN]");
	// TODO :...
	AIRTIO_DEBUG("getVolume [ END ]");
	return 0;
}

std::pair<float,float> airtio::Interface::getVolumeRange() const {
	std::unique_lock<std::mutex> lock(m_mutex);
	return std::make_pair(-120.0f, 0.0f);
}

void airtio::Interface::write(const std::vector<int16_t>& _value) {
	write(&_value[0], _value.size());
}

void airtio::Interface::write(const int16_t* _value, size_t _nbChunk) {
	std::unique_lock<std::mutex> lock(m_mutex);
	std::shared_ptr<airtalgo::EndPointWrite> algo = m_process->get<airtalgo::EndPointWrite>(0);
	if (algo == nullptr) {
		return;
	}
	algo->write(_value, _nbChunk);
}

// TODO : add API aCCess mutex for Read and write...
std::vector<int16_t> airtio::Interface::read(size_t _nbChunk) {
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

void airtio::Interface::read(const int16_t* _value, size_t _nbChunk) {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO :...
	
}

size_t airtio::Interface::size() const {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO :...
	return 0;
}

void airtio::Interface::setBufferSize(size_t _nbChunk) {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO :...
	
}

void airtio::Interface::setBufferSize(const std::chrono::duration<int64_t, std::micro>& _time) {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO :...
	
}

void airtio::Interface::clearInternalBuffer() {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO :...
	
}

std::chrono::system_clock::time_point airtio::Interface::getCurrentTime() const {
	std::unique_lock<std::mutex> lock(m_mutex);
	// TODO :...
	return std::chrono::system_clock::time_point();
	return std::chrono::system_clock::now();
}



void airtio::Interface::systemNewInputData(std::chrono::system_clock::time_point _time, void* _data, int32_t _nbChunk) {
	m_process->push(_time, _data, _nbChunk);
}

void airtio::Interface::systemNeedOutputData(std::chrono::system_clock::time_point _time, void* _data, int32_t _nbChunk, size_t _chunkSize) {
	size_t nbChunk2 = _nbChunk;
	m_process->pull(_time, _data, nbChunk2);//, _chunkSize);
#if 0
	// TODO : while enought data or fhush data:
	memset(_data, 0, _nbChunk*_chunkSize);
	AIRTIO_VERBOSE("        Interface DIRECT ");
	while(m_data.size()<_nbChunk*_chunkSize) {
		void* in = nullptr;
		size_t nbChunkIn = _nbChunk - m_data.size()/_chunkSize;
		void* out = nullptr;
		size_t nbChunkOut;
		if (nbChunkIn < 128) {
			nbChunkIn = 128;
		}
		// TODO : maybe remove this for input data ...
		for (int32_t iii=m_listAlgo.size()-1; iii >=0; --iii) {
			if (m_listAlgo[iii] != nullptr) {
				nbChunkIn = m_listAlgo[iii]->needInputData(nbChunkIn);
			}
		}
		if (nbChunkIn < 32) {
			nbChunkIn = 32;
		}
		//nbChunkIn *= 4;
		// get data from the upstream
		AIRTIO_VERBOSE("         * request " << nbChunkIn << " chunk");
		m_process->process(_time, in, nbChunkIn, out, nbChunkOut);
		AIRTIO_VERBOSE("         * get " << nbChunkOut << " chunk");
		if (nbChunkOut > 0) {
			size_t position = m_data.size();
			m_data.resize(m_data.size() + nbChunkOut*_chunkSize);
			memcpy(&m_data[position], out, nbChunkOut*_chunkSize);
		} else {
			// TODO : ERROR ...
			break;
		}
	}
	if (m_data.size()>=_nbChunk*_chunkSize) {
		AIRTIO_VERBOSE("         * copy needed data");
		memcpy(_data, &m_data[0], _nbChunk*_chunkSize);
		m_data.erase(m_data.begin(), m_data.begin()+_nbChunk*_chunkSize);
	} else {
		AIRTIO_VERBOSE("         * soft underflow");
		// ERROR
		m_data.clear();
	}
	/*
	process(_time, in, nbChunkIn, out, nbChunkOut);
	if (nbChunkIn!=nbChunkOut) {
		AIRTIO_ERROR(" wrong size : request=" << _nbChunk << " get=" << nbChunkOut);
		return;
	}
	memcpy(_data, out, _nbChunk*_chunkSize);
	*/
#endif
}
