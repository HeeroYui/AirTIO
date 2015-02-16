/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_IO_NODE_AEC_H__
#define __RIVER_IO_NODE_AEC_H__

#include <river/io/Node.h>
#include <river/Interface.h>

namespace river {
	namespace io {
		class Manager;
		class NodeAEC : public Node {
			protected:
				/**
				 * @brief Constructor
				 */
				NodeAEC(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
			public:
				static std::shared_ptr<NodeAEC> create(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
				/**
				 * @brief Destructor
				 */
				virtual ~NodeAEC();
			protected:
				virtual void start();
				virtual void stop();
				std::shared_ptr<river::Interface> m_interfaceMicrophone;
				std::shared_ptr<river::Interface> m_interfaceFeedBack;
				std::shared_ptr<river::Interface> createInput(float _freq,
				                                              const std::vector<audio::channel>& _map,
				                                              audio::format _format,
				                                              const std::string& _streamName,
				                                              const std::string& _name);
				void onDataReceivedMicrophone(const std::chrono::system_clock::time_point& _playTime,
				                              size_t _nbChunk,
				                              const std::vector<audio::channel>& _map,
				                              const void* _data,
				                              enum audio::format _type);
				
				void onDataReceivedFeedBack(const std::chrono::system_clock::time_point& _readTime,
				                            size_t _nbChunk,
				                            const std::vector<audio::channel>& _map,
				                            const void* _data,
				                            enum audio::format _type);
		};
	}
}

#endif

