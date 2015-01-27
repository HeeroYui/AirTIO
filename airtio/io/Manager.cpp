/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "Manager.h"
#include <memory>
#include "Node.h"

#undef __class__
#define __class__ "io::Manager"

std::shared_ptr<airtio::io::Manager> airtio::io::Manager::getInstance() {
	static std::shared_ptr<airtio::io::Manager> manager(new Manager());
	return manager;
}

std::shared_ptr<airtio::io::Node> airtio::io::Manager::getNode(const std::string& _streamName, bool _isInput) {
	for (size_t iii=0; iii< m_list.size(); ++iii) {
		std::shared_ptr<airtio::io::Node> tmppp = m_list[iii].lock();
		if (    tmppp!=nullptr
		     && _streamName == tmppp->getName()
		     && _isInput == tmppp->isInput()) {
			return tmppp;
		}
	}
	std::shared_ptr<airtio::io::Node> tmp = airtio::io::Node::create(_streamName, _isInput);
	m_list.push_back(tmp);
	return tmp;
}