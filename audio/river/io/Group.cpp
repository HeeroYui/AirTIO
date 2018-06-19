/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <audio/river/io/Group.hpp>
#include <audio/river/debug.hpp>
#include <audio/river/io/Node.hpp>
#include <audio/river/io/NodeAEC.hpp>
#include <audio/river/io/NodeOrchestra.hpp>
#include <audio/river/io/NodePortAudio.hpp>
#include <audio/river/io/Node.hpp>

void audio::river::io::Group::createFrom(const ejson::Document& _obj, const etk::String& _name) {
	RIVER_INFO("Create Group[" << _name << "] (START)    ___________________________");
	for (size_t iii=0; iii<_obj.size(); ++iii) {
		const ejson::Object tmpObject = _obj[iii].toObject();
		if (tmpObject.exist() == false) {
			continue;
		}
		etk::String groupName = tmpObject["group"].toString().get();
		if (groupName == _name) {
			RIVER_INFO("Add element in Group[" << _name << "]: " << _obj.getKey(iii));
			// get type : io
			etk::String ioType = tmpObject["io"].toString().get("error");
			#ifdef AUDIO_RIVER_BUILD_ORCHESTRA
				if (    ioType == "input"
				     || ioType == "output") {
					ememory::SharedPtr<audio::river::io::Node> tmp = audio::river::io::NodeOrchestra::create(_obj.getKey(iii), tmpObject);
					tmp->setGroup(sharedFromThis());
					m_list.pushBack(tmp);
				}
			#endif
			#ifdef AUDIO_RIVER_BUILD_PORTAUDIO
				if (    ioType == "PAinput"
				     || ioType == "PAoutput") {
					ememory::SharedPtr<audio::river::io::Node> tmp = audio::river::io::NodePortAudio::create(_obj.getKey(iii), tmpObject);
					tmp->setGroup(sharedFromThis());
					m_list.pushBack(tmp);
				}
			#endif
		}
	}
	// Link all the IO together : (not needed if one device ...
	// Note : The interlink work only for alsa (NOW) and with AirTAudio...
	if(m_list.size() > 1) {
		#ifdef AUDIO_RIVER_BUILD_ORCHESTRA
			ememory::SharedPtr<audio::river::io::NodeOrchestra> linkRef = ememory::dynamicPointerCast<audio::river::io::NodeOrchestra>(m_list[0]);
			for (size_t iii=1; iii<m_list.size(); ++iii) {
				if (m_list[iii] != null) {
					ememory::SharedPtr<audio::river::io::NodeOrchestra> link = ememory::dynamicPointerCast<audio::river::io::NodeOrchestra>(m_list[iii]);
					linkRef->m_interface.isMasterOf(link->m_interface);
				}
			}
		#endif
	}
	/*
	// manage Link Between Nodes :
	if (m_link != null) {
		RIVER_INFO("********   START LINK   ************");
		ememory::SharedPtr<audio::river::io::NodeOrchestra> link = ememory::dynamicPointerCast<audio::river::io::NodeOrchestra>(m_link);
		if (link == null) {
			RIVER_ERROR("Can not link 2 Interface with not the same type (reserved for HW interface)");
			return;
		}
		link->m_adac.isMasterOf(m_adac);
		// TODO : Add return ...
		RIVER_INFO("********   LINK might be done  ************");
	}
	*/
	RIVER_INFO("Create Group[" << _name << "] ( END )    ___________________________");
	RIVER_INFO("Group[" << _name << "] List elements : ");
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		if (m_list[iii] != null) {
			RIVER_INFO("    " << m_list[iii]->getName());
		}
	}
}


ememory::SharedPtr<audio::river::io::Node> audio::river::io::Group::getNode(const etk::String& _name) {
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		if (m_list[iii] != null) {
			if (m_list[iii]->getName() == _name) {
				return m_list[iii];
			}
		}
	}
	return ememory::SharedPtr<audio::river::io::Node>();
}

void audio::river::io::Group::start() {
	RIVER_ERROR("request start ");
	int32_t count = 0;
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		if (m_list[iii] != null) {
			count += m_list[iii]->getNumberOfInterface();
		}
	}
	RIVER_ERROR(" have " << count << " interfaces ...");
	if (count == 1) {
		RIVER_ERROR("GROUP :::::::::::: START() [START]");
		for (size_t iii=0; iii<m_list.size(); ++iii) {
			if (m_list[iii] != null) {
				m_list[iii]->start();
			}
		}
		RIVER_ERROR("GROUP :::::::::::: START() [DONE]");
	}
}

void audio::river::io::Group::stop() {
	RIVER_ERROR("request stop ");
	int32_t count = 0;
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		if (m_list[iii] != null) {
			count += m_list[iii]->getNumberOfInterface();
		}
	}
	RIVER_ERROR(" have " << count << " interfaces ...");
	if (count == 0) {
		RIVER_ERROR("GROUP :::::::::::: STOP() [START]");
		for (int32_t iii=m_list.size()-1; iii>=0; --iii) {
			if (m_list[iii] != null) {
				m_list[iii]->stop();
			}
		}
		RIVER_ERROR("GROUP :::::::::::: STOP() [DONE]");
	}
}

void audio::river::io::Group::generateDot(etk::FSNode& _node, bool _hardwareNode) {
	for (size_t iii=0; iii<m_list.size(); ++iii) {
		if (m_list[iii] != null) {
			if (m_list[iii]->isHarwareNode() == _hardwareNode) {
				m_list[iii]->generateDot(_node);
			}
		}
	}
}

