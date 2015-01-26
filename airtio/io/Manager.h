/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __AIRTIO_IO_MANAGER_H__
#define __AIRTIO_IO_MANAGER_H__

#include <string>
#include <vector>
#include <list>
#include <stdint.h>
#include <chrono>
#include <functional>
#include <airtalgo/format.h>
#include <airtalgo/channel.h>
#include <memory>

namespace airtio {
	namespace io {
		class Node;
		class Manager {
			private:
				/**
				 * @brief Constructor
				 */
				Manager() {};
			public:
				static std::shared_ptr<Manager> getInstance();
				/**
				 * @brief Destructor
				 */
				virtual ~Manager() {};
			private:
				std::vector<std::weak_ptr<airtio::io::Node> > m_list;
			public:
				std::shared_ptr<airtio::io::Node> getNode(const std::string& _streamName, bool _isInput);
				
		};
	}
}

#endif

