/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <audio/river/io/Node.h>
#include <audio/river/Interface.h>
#include <audio/drain/CircularBuffer.h>

namespace audio {
	namespace river {
		namespace io {
			class Manager;
			class NodeMuxer : public Node {
				protected:
					/**
					 * @brief Constructor
					 */
					NodeMuxer(const std::string& _name, const ejson::Object& _config);
				public:
					static std::shared_ptr<NodeMuxer> create(const std::string& _name, const ejson::Object& _config);
					/**
					 * @brief Destructor
					 */
					virtual ~NodeMuxer();
				protected:
					virtual void start();
					virtual void stop();
					std::shared_ptr<audio::river::Interface> m_interfaceInput1;
					std::shared_ptr<audio::river::Interface> m_interfaceInput2;
					std::shared_ptr<audio::river::Interface> createInput(float _freq,
					                                              const std::vector<audio::channel>& _map,
					                                              audio::format _format,
					                                              const std::string& _streamName,
					                                              const std::string& _name);
					void onDataReceivedInput1(const void* _data,
					                          const audio::Time& _time,
					                          size_t _nbChunk,
					                          enum audio::format _format,
					                          uint32_t _frequency,
					                          const std::vector<audio::channel>& _map);
					void onDataReceivedInput2(const void* _data,
					                          const audio::Time& _time,
					                          size_t _nbChunk,
					                          enum audio::format _format,
					                          uint32_t _frequency,
					                          const std::vector<audio::channel>& _map);
					std::vector<audio::channel> m_mapInput1;
					std::vector<audio::channel> m_mapInput2;
					audio::drain::CircularBuffer m_bufferInput1;
					audio::drain::CircularBuffer m_bufferInput2;
					audio::Duration m_sampleTime; //!< represent the sample time at the specify frequency.
					void process();
					void processMuxer(void* _dataMic, void* _dataFB, uint32_t _nbChunk, const audio::Time& _time);
					std::vector<uint8_t> m_data;
				public:
					virtual void generateDot(etk::FSNode& _node);
				private:
					void reorder(void* _output, uint32_t _nbChunk, void* _input, const std::vector<audio::channel>& _mapInput);
			};
		}
	}
}

