/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_IO_NODE_MUXER_H__
#define __RIVER_IO_NODE_MUXER_H__

#include <river/io/Node.h>
#include <river/Interface.h>
#include <river/CircularBuffer.h>

namespace river {
	namespace io {
		class Manager;
		class NodeMuxer : public Node {
			protected:
				/**
				 * @brief Constructor
				 */
				NodeMuxer(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config);
			public:
				static std11::shared_ptr<NodeMuxer> create(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config);
				/**
				 * @brief Destructor
				 */
				virtual ~NodeMuxer();
			protected:
				virtual void start();
				virtual void stop();
				std11::shared_ptr<river::Interface> m_interfaceInput1;
				std11::shared_ptr<river::Interface> m_interfaceInput2;
				std11::shared_ptr<river::Interface> createInput(float _freq,
				                                              const std::vector<audio::channel>& _map,
				                                              audio::format _format,
				                                              const std::string& _streamName,
				                                              const std::string& _name);
				void onDataReceivedInput1(const void* _data,
				                          const std11::chrono::system_clock::time_point& _time,
				                          size_t _nbChunk,
				                          enum audio::format _format,
				                          uint32_t _frequency,
				                          const std::vector<audio::channel>& _map);
				void onDataReceivedInput2(const void* _data,
				                          const std11::chrono::system_clock::time_point& _time,
				                          size_t _nbChunk,
				                          enum audio::format _format,
				                          uint32_t _frequency,
				                          const std::vector<audio::channel>& _map);
				std::vector<audio::channel> m_mapInput1;
				std::vector<audio::channel> m_mapInput2;
				river::CircularBuffer m_bufferInput1;
				river::CircularBuffer m_bufferInput2;
				std11::chrono::nanoseconds m_sampleTime; //!< represent the sample time at the specify frequency.
				void process();
				void processMuxer(void* _dataMic, void* _dataFB, uint32_t _nbChunk, const std11::chrono::system_clock::time_point& _time);
				std::vector<uint8_t> m_data;
			public:
				virtual void generateDot(etk::FSNode& _node);
			private:
				void reorder(void* _output, uint32_t _nbChunk, void* _input, const std::vector<audio::channel>& _mapInput);
		};
	}
}

#endif

