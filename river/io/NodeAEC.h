/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_IO_NODE_AEC_H__
#define __RIVER_IO_NODE_AEC_H__

#include <river/io/Node.h>
#include <river/Interface.h>
#include <river/CircularBuffer.h>

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
				void onDataReceivedMicrophone(const void* _data,
				                              const std::chrono::system_clock::time_point& _time,
				                              size_t _nbChunk,
				                              enum audio::format _format,
				                              uint32_t _frequency,
				                              const std::vector<audio::channel>& _map);
				void onDataReceivedFeedBack(const void* _data,
				                            const std::chrono::system_clock::time_point& _time,
				                            size_t _nbChunk,
				                            enum audio::format _format,
				                            uint32_t _frequency,
				                            const std::vector<audio::channel>& _map);
				river::CircularBuffer m_bufferMicrophone;
				river::CircularBuffer m_bufferFeedBack;
				std::chrono::nanoseconds m_sampleTime; //!< represent the sample time at the specify frequency.
				void process();
				void processAEC(void* _dataMic, void* _dataFB, uint32_t _nbChunk, const std::chrono::system_clock::time_point& _time);
		};
	}
}

#endif

