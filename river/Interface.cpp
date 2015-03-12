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
  m_node(),
  m_name("") {
	static uint32_t uid = 0;
	m_uid = uid++;
	
}

bool river::Interface::init(float _freq,
                            const std::vector<audio::channel>& _map,
                            audio::format _format,
                            const std11::shared_ptr<river::io::Node>& _node,
                            const std11::shared_ptr<const ejson::Object>& _config) {
	std::vector<audio::channel> map(_map);
	m_node = _node;
	m_config = _config;
	m_mode = river::modeInterface_unknow;
	std::string type = m_config->getStringValue("io", "error");
	m_name = _node->getName() + "__" + (_node->isInput()==true?"input":"output") + "__" + type;
	if (type == "output") {
		m_mode = river::modeInterface_output;
	} else if (type == "input") {
		m_mode = river::modeInterface_input;
	} else if (type == "feedback") {
		m_mode = river::modeInterface_feedback;
	}
	// register interface to be notify from the volume change.
	m_node->registerAsRemote(shared_from_this());
	
	if (map.size() == 0) {
		RIVER_INFO("Select auto map system ...");
		map = m_node->getInterfaceFormat().getMap();
		RIVER_INFO("    ==> " << map);
	}
	
	// Create convertion interface
	if (    m_node->isInput() == true
	     && m_mode == river::modeInterface_input) {
		m_process.setInputConfig(m_node->getInterfaceFormat());
		// Add volume only if the Low level has a volume (otherwise it will be added by the application)
		std11::shared_ptr<drain::VolumeElement> tmpVolume = m_node->getVolume();
		if (tmpVolume != nullptr) {
			// add all time the volume stage :
			std11::shared_ptr<drain::Volume> algo = drain::Volume::create();
			//algo->setInputFormat(m_node->getInterfaceFormat());
			algo->setName("volume");
			m_process.pushBack(algo);
			RIVER_INFO(" add volume for node");
			algo->addVolumeStage(tmpVolume);
		}
		m_process.setOutputConfig(drain::IOFormatInterface(map, _format, _freq));
	} else if (    m_node->isOutput() == true
	            && m_mode == river::modeInterface_output) {
		m_process.setInputConfig(drain::IOFormatInterface(map, _format, _freq));
		// Add volume only if the Low level has a volume (otherwise it will be added by the application)
		std11::shared_ptr<drain::VolumeElement> tmpVolume = m_node->getVolume();
		if (tmpVolume != nullptr) {
			// add all time the volume stage :
			std11::shared_ptr<drain::Volume> algo = drain::Volume::create();
			//algo->setOutputFormat(m_node->getInterfaceFormat());
			algo->setName("volume");
			m_process.pushBack(algo);
			RIVER_INFO(" add volume for node");
			algo->addVolumeStage(tmpVolume);
		}
		m_process.setOutputConfig(m_node->getInterfaceFormat());
	} else if (    m_node->isOutput() == true
	            && m_mode == river::modeInterface_feedback) {
		m_process.setInputConfig(m_node->getHarwareFormat());
		// note : feedback has no volume stage ...
		m_process.setOutputConfig(drain::IOFormatInterface(map, _format, _freq));
	} else {
		RIVER_ERROR("Can not link virtual interface with type : " << m_mode << " to a hardware interface " << (m_node->isInput()==true?"input":"output"));
		return false;
	}
	return true;
}

std11::shared_ptr<river::Interface> river::Interface::create(float _freq,
                                                             const std::vector<audio::channel>& _map,
                                                             audio::format _format,
                                                             const std11::shared_ptr<river::io::Node>& _node,
                                                             const std11::shared_ptr<const ejson::Object>& _config) {
	std11::shared_ptr<river::Interface> out = std11::shared_ptr<river::Interface>(new river::Interface());
	out->init(_freq, _map, _format, _node, _config);
	return out;
}

river::Interface::~Interface() {
	//stop(true, true);
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	//m_node->interfaceRemove(shared_from_this());
}
/*
bool river::Interface::hasEndPoint() {
	
}
*/
void river::Interface::setReadwrite() {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	m_process.removeAlgoDynamic();
	if (m_process.hasType<drain::EndPoint>() ) {
		RIVER_ERROR("Endpoint is already present ==> can not change");
		return;
	}
	if (m_node->isInput() == true) {
		m_process.removeIfLast<drain::EndPoint>();
		std11::shared_ptr<drain::EndPointRead> algo = drain::EndPointRead::create();
		m_process.pushBack(algo);
	} else {
		m_process.removeIfFirst<drain::EndPoint>();
		std11::shared_ptr<drain::EndPointWrite> algo = drain::EndPointWrite::create();
		m_process.pushFront(algo);
	}
}

void river::Interface::setOutputCallback(drain::playbackFunction _function) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	if (m_mode != river::modeInterface_output) {
		RIVER_ERROR("Can not set output endpoint on other than a output IO");
		return;
	}
	m_process.removeAlgoDynamic();
	m_process.removeIfFirst<drain::EndPoint>();
	std11::shared_ptr<drain::Algo> algo = drain::EndPointCallback::create(_function);
	m_process.pushFront(algo);
}

void river::Interface::setInputCallback(drain::recordFunction _function) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	if (m_mode == river::modeInterface_output) {
		RIVER_ERROR("Can not set output endpoint on other than a input or feedback IO");
		return;
	}
	m_process.removeAlgoDynamic();
	m_process.removeIfLast<drain::EndPoint>();
	std11::shared_ptr<drain::Algo> algo = drain::EndPointCallback::create(_function);
	m_process.pushBack(algo);
}

void river::Interface::setWriteCallback(drain::playbackFunctionWrite _function) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	if (m_mode != river::modeInterface_output) {
		RIVER_ERROR("Can not set output endpoint on other than a output IO");
		return;
	}
	m_process.removeAlgoDynamic();
	std11::shared_ptr<drain::EndPointWrite> algo = m_process.get<drain::EndPointWrite>(0);
	if (algo == nullptr) {
		return;
	}
	algo->setCallback(_function);
}

void river::Interface::start(const std11::chrono::system_clock::time_point& _time) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	RIVER_DEBUG("start [BEGIN]");
	m_process.updateInterAlgo();
	m_node->interfaceAdd(shared_from_this());
	RIVER_DEBUG("start [ END ]");
}

void river::Interface::stop(bool _fast, bool _abort) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	RIVER_DEBUG("stop [BEGIN]");
	m_node->interfaceRemove(shared_from_this());
	RIVER_DEBUG("stop [ END]");
}

void river::Interface::abort() {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	RIVER_DEBUG("abort [BEGIN]");
	// TODO :...
	RIVER_DEBUG("abort [ END ]");
}

bool river::Interface::setParameter(const std::string& _filter, const std::string& _parameter, const std::string& _value) {
	RIVER_DEBUG("setParameter [BEGIN] : '" << _filter << "':'" << _parameter << "':'" << _value << "'");
	bool out = false;
	if (    _filter == "volume"
	     && _parameter != "FLOW") {
		RIVER_ERROR("Interface is not allowed to modify '" << _parameter << "' Volume just allowed to modify 'FLOW' volume");
		return false;
	}
	std11::shared_ptr<drain::Algo> algo = m_process.get<drain::Algo>(_filter);
	if (algo == nullptr) {
		RIVER_ERROR("setParameter(" << _filter << ") ==> no filter named like this ...");
		return false;
	}
	out = algo->setParameter(_parameter, _value);
	RIVER_DEBUG("setParameter [ END ] : '" << out << "'");
	return out;
}
std::string river::Interface::getParameter(const std::string& _filter, const std::string& _parameter) const {
	RIVER_DEBUG("getParameter [BEGIN] : '" << _filter << "':'" << _parameter << "'");
	std::string out;
	std11::shared_ptr<const drain::Algo> algo = m_process.get<const drain::Algo>(_filter);
	if (algo == nullptr) {
		RIVER_ERROR("setParameter(" << _filter << ") ==> no filter named like this ...");
		return "[ERROR]";
	}
	out = algo->getParameter(_parameter);
	RIVER_DEBUG("getParameter [ END ] : '" << out << "'");
	return out;
}
std::string river::Interface::getParameterProperty(const std::string& _filter, const std::string& _parameter) const {
	RIVER_DEBUG("getParameterProperty [BEGIN] : '" << _filter << "':'" << _parameter << "'");
	std::string out;
	std11::shared_ptr<const drain::Algo> algo = m_process.get<const drain::Algo>(_filter);
	if (algo == nullptr) {
		RIVER_ERROR("setParameter(" << _filter << ") ==> no filter named like this ...");
		return "[ERROR]";
	}
	out = algo->getParameterProperty(_parameter);
	RIVER_DEBUG("getParameterProperty [ END ] : '" << out << "'");
	return out;
}

void river::Interface::write(const void* _value, size_t _nbChunk) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	m_process.updateInterAlgo();
	std11::shared_ptr<drain::EndPointWrite> algo = m_process.get<drain::EndPointWrite>(0);
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
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	m_process.updateInterAlgo();
	// TODO :...
	
}

size_t river::Interface::size() const {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	// TODO :...
	return 0;
}

void river::Interface::setBufferSize(size_t _nbChunk) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	m_process.updateInterAlgo();
	// TODO :...
	
}

void river::Interface::setBufferSize(const std11::chrono::microseconds& _time) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	m_process.updateInterAlgo();
	// TODO :...
	
}

void river::Interface::clearInternalBuffer() {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	m_process.updateInterAlgo();
	// TODO :...
	
}

std11::chrono::system_clock::time_point river::Interface::getCurrentTime() const {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	// TODO :...
	return std11::chrono::system_clock::time_point();
	return std11::chrono::system_clock::now();
}

void river::Interface::addVolumeGroup(const std::string& _name) {
	std11::unique_lock<std11::recursive_mutex> lock(m_mutex);
	RIVER_DEBUG("addVolumeGroup(" << _name << ")");
	std11::shared_ptr<drain::Volume> algo = m_process.get<drain::Volume>("volume");
	if (algo == nullptr) {
		m_process.removeAlgoDynamic();
		// add all time the volume stage :
		algo = drain::Volume::create();
		algo->setName("volume");
		if (m_node->isInput() == true) {
			m_process.pushFront(algo);
		} else {
			m_process.pushBack(algo);
		}
	}
	if (_name == "FLOW") {
		// Local volume name
		algo->addVolumeStage(std11::make_shared<drain::VolumeElement>(_name));
	} else {
		// get manager unique instance:
		std11::shared_ptr<river::io::Manager> mng = river::io::Manager::getInstance();
		algo->addVolumeStage(mng->getVolumeGroup(_name));
	}
}

void river::Interface::systemNewInputData(std11::chrono::system_clock::time_point _time, const void* _data, size_t _nbChunk) {
	std11::unique_lock<std11::recursive_mutex> lockProcess(m_mutex);
	void * tmpData = const_cast<void*>(_data);
	m_process.push(_time, tmpData, _nbChunk);
}

void river::Interface::systemNeedOutputData(std11::chrono::system_clock::time_point _time, void* _data, size_t _nbChunk, size_t _chunkSize) {
	std11::unique_lock<std11::recursive_mutex> lockProcess(m_mutex);
	//RIVER_INFO("time :                           " << _time);
	m_process.pull(_time, _data, _nbChunk, _chunkSize);
}

void river::Interface::systemVolumeChange() {
	std11::unique_lock<std11::recursive_mutex> lockProcess(m_mutex);
	std11::shared_ptr<drain::Volume> algo = m_process.get<drain::Volume>("volume");
	if (algo == nullptr) {
		return;
	}
	algo->volumeChange();
}

static void link(etk::FSNode& _node, const std::string& _first, const std::string& _op, const std::string& _second, bool _isLink=true) {
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

std::string river::Interface::getDotNodeName() const {
	if (m_mode == river::modeInterface_input) {
		return "API_" + etk::to_string(m_uid) + "_input";
	} else if (m_mode == river::modeInterface_feedback) {
		return "API_" + etk::to_string(m_uid) + "_feedback";
	} else if (m_mode == river::modeInterface_output) {
		return "API_" + etk::to_string(m_uid) + "_output";
	}
	return "error";
}

void river::Interface::generateDot(etk::FSNode& _node, const std::string& _nameIO, bool _isLink) {
	_node << "	subgraph clusterInterface_" << m_uid << " {\n";
	_node << "		color=orange;\n";
	_node << "		label=\"[" << m_uid << "] Interface : " << m_name << "\";\n";
	std::string nameIn;
	std::string nameOut;
	if (    m_mode == river::modeInterface_input
	     || m_mode == river::modeInterface_feedback) {
		m_process.generateDot(_node, 3, 10000+m_uid, nameIn, nameOut, false);
	} else {
		m_process.generateDot(_node, 3, 10000+m_uid, nameIn, nameOut, true);
	}
	
	
	if (    m_mode == river::modeInterface_input
	     || m_mode == river::modeInterface_feedback) {
		link(_node, _nameIO,                                           "->", nameIn, _isLink);
	} else {
		link(_node, _nameIO,                                            "<-", nameOut, _isLink);
	}
	_node << "		node [shape=Mdiamond];\n";
	if (m_mode == river::modeInterface_input) {
		_node << "			" << getDotNodeName() << " [ label=\"API\\nINPUT\" ];\n";
		link(_node, nameOut, "->", getDotNodeName());
	} else if (m_mode == river::modeInterface_feedback) {
		_node << "			" << getDotNodeName() << " [ label=\"API\\nFEEDBACK\" ];\n";
		link(_node, nameOut, "->", getDotNodeName());
	} else if (m_mode == river::modeInterface_output) {
		_node << "			" << getDotNodeName() << " [ label=\"API\\nOUTPUT\" ];\n";
		link(_node, nameIn, "<-", getDotNodeName());
	}
	_node << "	}\n	\n";
}

