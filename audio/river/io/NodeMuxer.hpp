/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <audio/river/io/Node.hpp>
#include <audio/river/Interface.hpp>
#include <audio/drain/CircularBuffer.hpp>

namespace audio {
	namespace river {
		namespace io {
			class Manager;
			class NodeMuxer : public Node {
				protected:
					/**
					 * @brief Constructor
					 */
					NodeMuxer(const etk::String& _name, const ejson::Object& _config);
				public:
					static ememory::SharedPtr<NodeMuxer> create(const etk::String& _name, const ejson::Object& _config);
					/**
					 * @brief Destructor
					 */
					virtual ~NodeMuxer();
				protected:
					virtual void start();
					virtual void stop();
					ememory::SharedPtr<audio::river::Interface> m_interfaceInput1;
					ememory::SharedPtr<audio::river::Interface> m_interfaceInput2;
					ememory::SharedPtr<audio::river::Interface> createInput(float _freq,
					                                              const etk::Vector<audio::channel>& _map,
					                                              audio::format _format,
					                                              const etk::String& _streamName,
					                                              const etk::String& _name);
					void onDataReceivedInput1(const void* _data,
					                          const audio::Time& _time,
					                          size_t _nbChunk,
					                          enum audio::format _format,
					                          uint32_t _frequency,
					                          const etk::Vector<audio::channel>& _map);
					void onDataReceivedInput2(const void* _data,
					                          const audio::Time& _time,
					                          size_t _nbChunk,
					                          enum audio::format _format,
					                          uint32_t _frequency,
					                          const etk::Vector<audio::channel>& _map);
					etk::Vector<audio::channel> m_mapInput1;
					etk::Vector<audio::channel> m_mapInput2;
					audio::drain::CircularBuffer m_bufferInput1;
					audio::drain::CircularBuffer m_bufferInput2;
					audio::Duration m_sampleTime; //!< represent the sample time at the specify frequency.
					void process();
					void processMuxer(void* _dataMic, void* _dataFB, uint32_t _nbChunk, const audio::Time& _time);
					etk::Vector<uint8_t> m_data;
				public:
					virtual void generateDot(ememory::SharedPtr<etk::io::Interface>& _io);
				private:
					void reorder(void* _output, uint32_t _nbChunk, void* _input, const etk::Vector<audio::channel>& _mapInput);
			};
		}
	}
}

