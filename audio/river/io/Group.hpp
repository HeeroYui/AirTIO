/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#pragma once

#include <string>
#include <vector>
#include <ejson/ejson.hpp>
#include <etk/os/FSNode.hpp>

namespace audio {
	namespace river {
		namespace io {
			class Node;
			class Manager;
			/**
			 * @brief Group is contituate to manage some input and output in the same start and stop.
			 * It link N interface in a group. The start and the sopt is requested in Node inside the
			 * group they will start and stop when the first start is requested and stop when the last
			 * is stopped.
			 * @note For the Alsa interface a low level link is availlable with AirTAudio for Alsa (One thread)
			 */
			class Group : public ememory::EnableSharedFromThis<Group> {
				public:
					/**
					 * @brief Contructor. No special thing to do.
					 */
					Group() {}
					/**
					 * @brief Destructor
					 */
					~Group() = default;
				private: 
					std::vector< ememory::SharedPtr<Node> > m_list; //!< List of all node in the group
				public:
					/**
					 * @brief Create a group with all node needed to syncronize together
					 * @param[in] _obj json document to create all the node in the group named _name
					 * @param[in] _name Name of the group to create
					 */
					void createFrom(const ejson::Document& _obj, const std::string& _name);
					/**
					 * @brief Get a node in the group (if the node is not in the group nothing append).
					 * @param[in] _name Name of the node requested.
					 * @return nullptr The node named _name was not found.
					 * @return pointer The node was find in this group.
					 */
					ememory::SharedPtr<audio::river::io::Node> getNode(const std::string& _name);
					/**
					 * @brief Start the group.
					 * @note all sub-node will be started.
					 */
					void start();
					/**
					 * @brief Stop the group.
					 * @note All sub-node will be stopped at the reserve order that they start.
					 */
					void stop();
					/**
					 * @brief Create the dot in the FileNode stream.
					 * @param[in,out] _node File node to write data.
					 * @param[in] _hardwareNode true if user want only display the hardware
					 *                          node and not the software node. false The oposite.
					 */
					void generateDot(etk::FSNode& _node, bool _hardwareNode);
			};
		}
	}
}

