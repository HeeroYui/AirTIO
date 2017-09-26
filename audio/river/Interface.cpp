/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <audio/river/debug.hpp>
#include <audio/river/Interface.hpp>
#include <audio/river/io/Node.hpp>
#include <audio/drain/EndPointCallback.hpp>
#include <audio/drain/EndPointWrite.hpp>
#include <audio/drain/EndPointRead.hpp>
#include <audio/drain/Volume.hpp>

audio::river::Interface::Interface(void) :
  m_node(),
  m_name("") {
	static uint32_t uid = 0;
	m_uid = uid++;
	
}

bool audio::river::Interface::init(float _freq,
                                   const etk::Vector<audio::channel>& _map,
                                   audio::format _format,
                                   ememory::SharedPtr<audio::river::io::Node> _node,
                                   const ejson::Object& _config) {
	etk::Vector<audio::channel> map(_map);
	m_node = _node;
	m_config = _config;
	m_mode = audio::river::modeInterface_unknow;
	etk::String type = m_config["io"].toString().get("error");
	static int32_t uid=0;
	m_name = _node->getName() + "__" + (_node->isInput()==true?"input":"output") + "__" + type + "__" + etk::toString(uid++);
	if (type == "output") {
		m_mode = audio::river::modeInterface_output;
	} else if (type == "input") {
		m_mode = audio::river::modeInterface_input;
	} else if (type == "feedback") {
		m_mode = audio::river::modeInterface_feedback;
	}
	// register interface to be notify from the volume change.
	m_node->registerAsRemote(sharedFromThis());
	
	if (map.size() == 0) {
		RIVER_INFO("Select auto map system ...");
		map = m_node->getInterfaceFormat().getMap();
		RIVER_INFO("    ==> " << map);
	}
	
	// Create convertion interface
	if (    m_node->isInput() == true
	     && m_mode == audio::river::modeInterface_input) {
		m_process.setInputConfig(m_node->getInterfaceFormat());
		// Add volume only if the Low level has a volume (otherwise it will be added by the application)
		ememory::SharedPtr<audio::drain::VolumeElement> tmpVolume = m_node->getVolume();
		if (tmpVolume != nullptr) {
			// add all time the volume stage :
			ememory::SharedPtr<audio::drain::Volume> algo = audio::drain::Volume::create();
			//algo->setInputFormat(m_node->getInterfaceFormat());
			algo->setName("volume");
			m_process.pushBack(algo);
			RIVER_INFO(" add volume for node");
			algo->addVolumeStage(tmpVolume);
		}
		m_process.setOutputConfig(audio::drain::IOFormatInterface(map, _format, _freq));
	} else if (    m_node->isOutput() == true
	            && m_mode == audio::river::modeInterface_output) {
		m_process.setInputConfig(audio::drain::IOFormatInterface(map, _format, _freq));
		// Add volume only if the Low level has a volume (otherwise it will be added by the application)
		ememory::SharedPtr<audio::drain::VolumeElement> tmpVolume = m_node->getVolume();
		if (tmpVolume != nullptr) {
			// add all time the volume stage :
			ememory::SharedPtr<audio::drain::Volume> algo = audio::drain::Volume::create();
			//algo->setOutputFormat(m_node->getInterfaceFormat());
			algo->setName("volume");
			m_process.pushBack(algo);
			RIVER_INFO(" add volume for node");
			algo->addVolumeStage(tmpVolume);
		}
		m_process.setOutputConfig(m_node->getInterfaceFormat());
	} else if (    m_node->isOutput() == true
	            && m_mode == audio::river::modeInterface_feedback) {
		m_process.setInputConfig(m_node->getHarwareFormat());
		// note : feedback has no volume stage ...
		m_process.setOutputConfig(audio::drain::IOFormatInterface(map, _format, _freq));
	} else {
		RIVER_ERROR("Can not link virtual interface with type : " << m_mode << " to a hardware interface " << (m_node->isInput()==true?"input":"output"));
		return false;
	}
	return true;
}

ememory::SharedPtr<audio::river::Interface> audio::river::Interface::create(float _freq,
                                                                            const etk::Vector<audio::channel>& _map,
                                                                            audio::format _format,
                                                                            const ememory::SharedPtr<audio::river::io::Node>& _node,
                                                                            const ejson::Object& _config) {
	ememory::SharedPtr<audio::river::Interface> out = ememory::SharedPtr<audio::river::Interface>(new audio::river::Interface());
	out->init(_freq, _map, _format, _node, _config);
	return out;
}

audio::river::Interface::~Interface() {
	//stop(true, true);
	ethread::RecursiveLock lock(m_mutex);
	//m_node->interfaceRemove(sharedFromThis());
}
/*
bool audio::river::Interface::hasEndPoint() {
	
}
*/
void audio::river::Interface::setReadwrite() {
	ethread::RecursiveLock lock(m_mutex);
	m_process.removeAlgoDynamic();
	if (m_process.hasType<audio::drain::EndPoint>() ) {
		RIVER_ERROR("Endpoint is already present ==> can not change");
		return;
	}
	RIVER_WARNING("Add output ReadWrite");
	if (m_node->isInput() == true) {
		m_process.removeIfLast<audio::drain::EndPoint>();
		ememory::SharedPtr<audio::drain::EndPointRead> algo = audio::drain::EndPointRead::create();
		m_process.pushBack(algo);
	} else {
		m_process.removeIfFirst<audio::drain::EndPoint>();
		ememory::SharedPtr<audio::drain::EndPointWrite> algo = audio::drain::EndPointWrite::create();
		m_process.pushFront(algo);
	}
}

void audio::river::Interface::setOutputCallback(audio::drain::playbackFunction _function) {
	ethread::RecursiveLock lock(m_mutex);
	if (m_mode != audio::river::modeInterface_output) {
		RIVER_ERROR("Can not set output endpoint on other than a output IO");
		return;
	}
	RIVER_WARNING("Add output callback");
	m_process.removeAlgoDynamic();
	m_process.removeIfFirst<audio::drain::EndPoint>();
	ememory::SharedPtr<audio::drain::Algo> algo = audio::drain::EndPointCallback::create(_function);
	m_process.pushFront(algo);
}

void audio::river::Interface::setInputCallback(audio::drain::recordFunction _function) {
	ethread::RecursiveLock lock(m_mutex);
	if (m_mode == audio::river::modeInterface_output) {
		RIVER_ERROR("Can not set output endpoint on other than a input or feedback IO");
		return;
	}
	RIVER_WARNING("Add input callback");
	m_process.removeAlgoDynamic();
	m_process.removeIfLast<audio::drain::EndPoint>();
	ememory::SharedPtr<audio::drain::Algo> algo = audio::drain::EndPointCallback::create(_function);
	m_process.pushBack(algo);
}

void audio::river::Interface::setWriteCallback(audio::drain::playbackFunctionWrite _function) {
	ethread::RecursiveLock lock(m_mutex);
	if (m_mode != audio::river::modeInterface_output) {
		RIVER_ERROR("Can not set output endpoint on other than a output IO");
		return;
	}
	RIVER_WARNING("Add output Write");
	m_process.removeAlgoDynamic();
	ememory::SharedPtr<audio::drain::EndPointWrite> algo = m_process.get<audio::drain::EndPointWrite>(0);
	if (algo == nullptr) {
		return;
	}
	algo->setCallback(_function);
}

void audio::river::Interface::start(const audio::Time& _time) {
	ethread::RecursiveLock lock(m_mutex);
	RIVER_DEBUG("start [BEGIN]");
	m_process.updateInterAlgo();
	m_node->interfaceAdd(sharedFromThis());
	RIVER_DEBUG("start [ END ]");
}

void audio::river::Interface::stop(bool _fast, bool _abort) {
	ethread::RecursiveLock lock(m_mutex);
	RIVER_DEBUG("stop [BEGIN]");
	m_node->interfaceRemove(sharedFromThis());
	RIVER_DEBUG("stop [ END]");
}

void audio::river::Interface::abort() {
	ethread::RecursiveLock lock(m_mutex);
	RIVER_DEBUG("abort [BEGIN]");
	// TODO :...
	RIVER_DEBUG("abort [ END ]");
}

bool audio::river::Interface::setParameter(const etk::String& _filter, const etk::String& _parameter, const etk::String& _value) {
	RIVER_DEBUG("setParameter [BEGIN] : '" << _filter << "':'" << _parameter << "':'" << _value << "'");
	bool out = false;
	if (    _filter == "volume"
	     && _parameter != "FLOW") {
		RIVER_ERROR("Interface is not allowed to modify '" << _parameter << "' Volume just allowed to modify 'FLOW' volume");
		return false;
	}
	ememory::SharedPtr<audio::drain::Algo> algo = m_process.get<audio::drain::Algo>(_filter);
	if (algo == nullptr) {
		RIVER_ERROR("setParameter(" << _filter << ") ==> no filter named like this ...");
		return false;
	}
	out = algo->setParameter(_parameter, _value);
	RIVER_DEBUG("setParameter [ END ] : '" << out << "'");
	return out;
}
etk::String audio::river::Interface::getParameter(const etk::String& _filter, const etk::String& _parameter) const {
	RIVER_DEBUG("getParameter [BEGIN] : '" << _filter << "':'" << _parameter << "'");
	etk::String out;
	ememory::SharedPtr<const audio::drain::Algo> algo = m_process.get<const audio::drain::Algo>(_filter);
	if (algo == nullptr) {
		RIVER_ERROR("setParameter(" << _filter << ") ==> no filter named like this ...");
		return "[ERROR]";
	}
	out = algo->getParameter(_parameter);
	RIVER_DEBUG("getParameter [ END ] : '" << out << "'");
	return out;
}
etk::String audio::river::Interface::getParameterProperty(const etk::String& _filter, const etk::String& _parameter) const {
	RIVER_DEBUG("getParameterProperty [BEGIN] : '" << _filter << "':'" << _parameter << "'");
	etk::String out;
	ememory::SharedPtr<const audio::drain::Algo> algo = m_process.get<const audio::drain::Algo>(_filter);
	if (algo == nullptr) {
		RIVER_ERROR("setParameter(" << _filter << ") ==> no filter named like this ...");
		return "[ERROR]";
	}
	out = algo->getParameterProperty(_parameter);
	RIVER_DEBUG("getParameterProperty [ END ] : '" << out << "'");
	return out;
}

void audio::river::Interface::write(const void* _value, size_t _nbChunk) {
	ethread::RecursiveLock lock(m_mutex);
	m_process.updateInterAlgo();
	ememory::SharedPtr<audio::drain::EndPointWrite> algo = m_process.get<audio::drain::EndPointWrite>(0);
	if (algo == nullptr) {
		return;
	}
	algo->write(_value, _nbChunk);
}

#if 0
// TODO : add API aCCess mutex for Read and write...
etk::Vector<int16_t> audio::river::Interface::read(size_t _nbChunk) {
	// TODO :...
	etk::Vector<int16_t> data;
	/*
	data.resize(_nbChunk*m_map.size(), 0);
	m_mutex.lock();
	int32_t nbChunkBuffer = m_circularBuffer.size() / m_map.size();
	m_mutex.unlock();
	while (nbChunkBuffer < _nbChunk) {
		ethread::sleepMilliSeconds((1));
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

void audio::river::Interface::read(void* _value, size_t _nbChunk) {
	ethread::RecursiveLock lock(m_mutex);
	m_process.updateInterAlgo();
	// TODO :...
	
}

size_t audio::river::Interface::size() const {
	ethread::RecursiveLock lock(m_mutex);
	// TODO :...
	return 0;
}





void audio::river::Interface::setBufferSize(size_t _nbChunk) {
	ethread::RecursiveLock lock(m_mutex);
	if (m_node->isInput() == true) {
		ememory::SharedPtr<audio::drain::EndPointRead> algo = m_process.get<audio::drain::EndPointRead>(m_process.size()-1);
		if (algo == nullptr) {
			RIVER_ERROR("Request set buffer size for Interface that is not READ or WRITE mode ...");
			return;
		}
		algo->setBufferSize(_nbChunk);
		return;
	}
	ememory::SharedPtr<audio::drain::EndPointWrite> algo = m_process.get<audio::drain::EndPointWrite>(0);
	if (algo == nullptr) {
		RIVER_ERROR("Request set buffer size for Interface that is not READ or WRITE mode ...");
		return;
	}
	algo->setBufferSize(_nbChunk);
}

void audio::river::Interface::setBufferSize(const echrono::microseconds& _time) {
	ethread::RecursiveLock lock(m_mutex);
	if (m_node->isInput() == true) {
		ememory::SharedPtr<audio::drain::EndPointRead> algo = m_process.get<audio::drain::EndPointRead>(m_process.size()-1);
		if (algo == nullptr) {
			RIVER_ERROR("Request set buffer size for Interface that is not READ or WRITE mode ...");
			return;
		}
		algo->setBufferSize(_time);
		return;
	}
	ememory::SharedPtr<audio::drain::EndPointWrite> algo = m_process.get<audio::drain::EndPointWrite>(0);
	if (algo == nullptr) {
		RIVER_ERROR("Request set buffer size for Interface that is not READ or WRITE mode ...");
		return;
	}
	algo->setBufferSize(_time);
}

size_t audio::river::Interface::getBufferSize() {
	ethread::RecursiveLock lock(m_mutex);
	if (m_node->isInput() == true) {
		ememory::SharedPtr<audio::drain::EndPointRead> algo = m_process.get<audio::drain::EndPointRead>(m_process.size()-1);
		if (algo == nullptr) {
			RIVER_ERROR("Request get buffer size for Interface that is not READ or WRITE mode ...");
			return 0;
		}
		return algo->getBufferSize();
	}
	ememory::SharedPtr<audio::drain::EndPointWrite> algo = m_process.get<audio::drain::EndPointWrite>(0);
	if (algo == nullptr) {
		RIVER_ERROR("Request get buffer size for Interface that is not READ or WRITE mode ...");
		return 0;
	}
	return algo->getBufferSize();
}

echrono::microseconds audio::river::Interface::getBufferSizeMicrosecond() {
	ethread::RecursiveLock lock(m_mutex);
	if (m_node->isInput() == true) {
		ememory::SharedPtr<audio::drain::EndPointRead> algo = m_process.get<audio::drain::EndPointRead>(m_process.size()-1);
		if (algo == nullptr) {
			RIVER_ERROR("Request get buffer size for Interface that is not READ or WRITE mode ...");
			return echrono::microseconds(0);
		}
		return algo->getBufferSizeMicrosecond();
	}
	ememory::SharedPtr<audio::drain::EndPointWrite> algo = m_process.get<audio::drain::EndPointWrite>(0);
	if (algo == nullptr) {
		RIVER_ERROR("Request get buffer size for Interface that is not READ or WRITE mode ...");
		return echrono::microseconds(0);
	}
	return algo->getBufferSizeMicrosecond();
}

size_t audio::river::Interface::getBufferFillSize() {
	ethread::RecursiveLock lock(m_mutex);
	if (m_node->isInput() == true) {
		ememory::SharedPtr<audio::drain::EndPointRead> algo = m_process.get<audio::drain::EndPointRead>(m_process.size()-1);
		if (algo == nullptr) {
			RIVER_ERROR("Request get buffer size for Interface that is not READ or WRITE mode ...");
			return 0;
		}
		return algo->getBufferFillSize();
	}
	ememory::SharedPtr<audio::drain::EndPointWrite> algo = m_process.get<audio::drain::EndPointWrite>(0);
	if (algo == nullptr) {
		RIVER_ERROR("Request get buffer size for Interface that is not READ or WRITE mode ...");
		return 0;
	}
	return algo->getBufferFillSize();
	
}

echrono::microseconds audio::river::Interface::getBufferFillSizeMicrosecond() {
	ethread::RecursiveLock lock(m_mutex);
	if (m_node->isInput() == true) {
		ememory::SharedPtr<audio::drain::EndPointRead> algo = m_process.get<audio::drain::EndPointRead>(m_process.size()-1);
		if (algo == nullptr) {
			RIVER_ERROR("Request get buffer size for Interface that is not READ or WRITE mode ...");
			return echrono::microseconds(0);
		}
		return algo->getBufferFillSizeMicrosecond();
	}
	ememory::SharedPtr<audio::drain::EndPointWrite> algo = m_process.get<audio::drain::EndPointWrite>(0);
	if (algo == nullptr) {
		RIVER_ERROR("Request get buffer size for Interface that is not READ or WRITE mode ...");
		return echrono::microseconds(0);
	}
	return algo->getBufferFillSizeMicrosecond();
}



void audio::river::Interface::clearInternalBuffer() {
	ethread::RecursiveLock lock(m_mutex);
	m_process.updateInterAlgo();
	// TODO :...
	
}

audio::Time audio::river::Interface::getCurrentTime() const {
	ethread::RecursiveLock lock(m_mutex);
	// TODO :...
	return audio::Time();
	return audio::Time::now();
}

void audio::river::Interface::addVolumeGroup(const etk::String& _name) {
	ethread::RecursiveLock lock(m_mutex);
	RIVER_DEBUG("addVolumeGroup(" << _name << ")");
	ememory::SharedPtr<audio::drain::Volume> algo = m_process.get<audio::drain::Volume>("volume");
	if (algo == nullptr) {
		m_process.removeAlgoDynamic();
		// add all time the volume stage :
		algo = audio::drain::Volume::create();
		algo->setName("volume");
		if (m_node->isInput() == true) {
			m_process.pushFront(algo);
		} else {
			m_process.pushBack(algo);
		}
	}
	if (_name == "FLOW") {
		// Local volume name
		algo->addVolumeStage(ememory::makeShared<audio::drain::VolumeElement>(_name));
	} else {
		// get manager unique instance:
		ememory::SharedPtr<audio::river::io::Manager> mng = audio::river::io::Manager::getInstance();
		algo->addVolumeStage(mng->getVolumeGroup(_name));
	}
}

void audio::river::Interface::systemNewInputData(audio::Time _time, const void* _data, size_t _nbChunk) {
	ethread::RecursiveLock lockProcess(m_mutex);
	void * tmpData = const_cast<void*>(_data);
	m_process.push(_time, tmpData, _nbChunk);
}

void audio::river::Interface::systemNeedOutputData(audio::Time _time, void* _data, size_t _nbChunk, size_t _chunkSize) {
	ethread::RecursiveLock lockProcess(m_mutex);
	//RIVER_INFO("time :                           " << _time);
	m_process.pull(_time, _data, _nbChunk, _chunkSize);
}

void audio::river::Interface::systemVolumeChange() {
	ethread::RecursiveLock lockProcess(m_mutex);
	ememory::SharedPtr<audio::drain::Volume> algo = m_process.get<audio::drain::Volume>("volume");
	if (algo == nullptr) {
		return;
	}
	algo->volumeChange();
}

static void link(etk::FSNode& _node, const etk::String& _first, const etk::String& _op, const etk::String& _second, bool _isLink=true) {
	if (_op == "->") {
		if (_isLink) {
			_node << "			" << _first << " -> " << _second << ";\n";
		} else {
			_node << "			" << _first << " -> " << _second << " [style=dashed];\n";
		}
	} else if (_op == "<-") {
		_node << "			" << _first << " -> " <<_second<< " [color=transparent];\n";
		if (_isLink) {
			_node << "			" << _second << " -> " << _first << " [constraint=false];\n";
		} else {
			_node << "			" << _second << " -> " << _first << " [constraint=false, style=dashed];\n";
		}
	}
}

etk::String audio::river::Interface::getDotNodeName() const {
	if (m_mode == audio::river::modeInterface_input) {
		return "API_" + etk::toString(m_uid) + "_input";
	} else if (m_mode == audio::river::modeInterface_feedback) {
		return "API_" + etk::toString(m_uid) + "_feedback";
	} else if (m_mode == audio::river::modeInterface_output) {
		return "API_" + etk::toString(m_uid) + "_output";
	}
	return "error";
}

void audio::river::Interface::generateDot(etk::FSNode& _node, const etk::String& _nameIO, bool _isLink) {
	_node << "	subgraph clusterInterface_" << m_uid << " {\n";
	_node << "		color=orange;\n";
	_node << "		label=\"[" << m_uid << "] Interface : " << m_name << "\";\n";
	etk::String nameIn;
	etk::String nameOut;
	if (    m_mode == audio::river::modeInterface_input
	     || m_mode == audio::river::modeInterface_feedback) {
		m_process.generateDot(_node, 3, 10000+m_uid, nameIn, nameOut, false);
	} else {
		m_process.generateDot(_node, 3, 10000+m_uid, nameOut, nameIn, true);
	}
	
	
	if (    m_mode == audio::river::modeInterface_input
	     || m_mode == audio::river::modeInterface_feedback) {
		link(_node, _nameIO,                                           "->", nameIn, _isLink);
	} else {
		link(_node, _nameIO,                                            "<-", nameOut, _isLink);
	}
	_node << "		node [shape=Mdiamond];\n";
	if (m_mode == audio::river::modeInterface_input) {
		_node << "			" << getDotNodeName() << " [ label=\"API\\nINPUT\" ];\n";
		link(_node, nameOut, "->", getDotNodeName());
	} else if (m_mode == audio::river::modeInterface_feedback) {
		_node << "			" << getDotNodeName() << " [ label=\"API\\nFEEDBACK\" ];\n";
		link(_node, nameOut, "->", getDotNodeName());
	} else if (m_mode == audio::river::modeInterface_output) {
		_node << "			" << getDotNodeName() << " [ label=\"API\\nOUTPUT\" ];\n";
		link(_node, nameIn, "<-", getDotNodeName());
	}
	_node << "	}\n	\n";
}

