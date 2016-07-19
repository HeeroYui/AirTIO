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
			class NodeAEC : public Node {
				protected:
					/**
					 * @brief Constructor
					 */
					NodeAEC(const std::string& _name, const ejson::Object& _config);
				public:
					/**
					 * @brief Factory of this Virtual Node.
					 * @param[in] _name Name of the node.
					 * @param[in] _config Configuration of the node.
					 */
					static ememory::SharedPtr<NodeAEC> create(const std::string& _name, const ejson::Object& _config);
					/**
					 * @brief Destructor
					 */
					virtual ~NodeAEC();
				protected:
					virtual void start();
					virtual void stop();
					ememory::SharedPtr<audio::river::Interface> m_interfaceMicrophone; //!< Interface on the Microphone.
					ememory::SharedPtr<audio::river::Interface> m_interfaceFeedBack; //!< Interface on the feedback of speaker.
					/**
					 * @brief Internal: create an input with the specific parameter:
					 * @param[in] _freq Frequency.
					 * @param[in] _map Channel map organization.
					 * @param[in] _format Sample format
					 * @param[in] _streamName 
					 * @param[in] _name 
					 * @return Interfae Pointer.
					 */
					ememory::SharedPtr<audio::river::Interface> createInput(float _freq,
					                                                const std::vector<audio::channel>& _map,
					                                                audio::format _format,
					                                                const std::string& _streamName,
					                                                const std::string& _name);
					/**
					 * @brief Stream data input callback
					 * @todo : copy doc ..
					 */
					void onDataReceivedMicrophone(const void* _data,
					                              const audio::Time& _time,
					                              size_t _nbChunk,
					                              enum audio::format _format,
					                              uint32_t _frequency,
					                              const std::vector<audio::channel>& _map);
					/**
					 * @brief Stream data input callback
					 * @todo : copy doc ..
					 */
					void onDataReceivedFeedBack(const void* _data,
					                            const audio::Time& _time,
					                            size_t _nbChunk,
					                            enum audio::format _format,
					                            uint32_t _frequency,
					                            const std::vector<audio::channel>& _map);
				protected:
					audio::drain::CircularBuffer m_bufferMicrophone; //!< temporary buffer to synchronize data.
					audio::drain::CircularBuffer m_bufferFeedBack; //!< temporary buffer to synchronize data.
					audio::Duration m_sampleTime; //!< represent the sample time at the specify frequency.
					/**
					 * @brief Process synchronization on the 2 flow.
					 */
					void process();
					/**
					 * @brief Process algorithm on the current 2 syncronize flow.
					 * @param[in] _dataMic Pointer in the Microphione interface.
					 * @param[in] _dataFB Pointer on the beedback buffer.
					 * @param[in] _nbChunk Number of chunk to process.
					 * @param[in] _time Time on the firsta sample that data has been captured.
					 * @return 
					 */
					void processAEC(void* _dataMic, void* _dataFB, uint32_t _nbChunk, const audio::Time& _time);
				public:
					virtual void generateDot(etk::FSNode& _node);
				private:
					int32_t m_nbChunk;
					int32_t m_gainValue;
					int32_t m_sampleCount;
					
					int32_t m_P_attaqueTime; //ms
					int32_t m_P_releaseTime; //ms
					int32_t m_P_minimumGain; // %
					int32_t m_P_threshold; // %
					int32_t m_P_latencyTime; // ms
					
			};
		}
	}
}

