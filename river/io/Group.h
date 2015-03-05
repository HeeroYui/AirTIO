/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_IO_GROUP_H__
#define __RIVER_IO_GROUP_H__

#include <string>
#include <vector>
#include <ejson/ejson.h>
#include <etk/os/FSNode.h>

namespace river {
	namespace io {
		class Node;
		class Manager;
		class Group : public std11::enable_shared_from_this<Group> {
			public:
				Group() {}
				~Group() {}
			private:
				std::vector< std11::shared_ptr<Node> > m_list;
			public:
				void createFrom(const ejson::Document& _obj, const std::string& _name);
				std11::shared_ptr<river::io::Node> getNode(const std::string& _name);
				void start();
				void stop();
				void generateDot(etk::FSNode& _node, bool _hardwareNode);
		};
	}
}

#endif

