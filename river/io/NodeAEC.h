/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_IO_NODE_AEC_H__
#define __RIVER_IO_NODE_AEC_H__

#include <river/io/Node.h>
#include <river/Interface.h>
#include <drain/CircularBuffer.h>

namespace river {
	namespace io {
		class Manager;
		class NodeAEC : public Node {
			protected:
				/**
				 * @brief Constructor
				 */
				NodeAEC(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config);
			public:
				/**
				 * @brief Factory of this Virtual Node.
				 * @param[in] _name Name of the node.
				 * @param[in] _config Configuration of the node.
				 */
				static std11::shared_ptr<NodeAEC> create(const std::string& _name, const std11::shared_ptr<const ejson::Object>& _config);
				/**
				 * @brief Destructor
				 */
				virtual ~NodeAEC();
			protected:
				virtual void start();
				virtual void stop();
				std11::shared_ptr<river::Interface> m_interfaceMicrophone; //!< Interface on the Microphone.
				std11::shared_ptr<river::Interface> m_interfaceFeedBack; //!< Interface on the feedback of speaker.
				/**
				 * @brief Internal: create an input with the specific parameter:
				 * @param[in] _freq Frequency.
				 * @param[in] _map Channel map organization.
				 * @param[in] _format Sample format
				 * @param[in] _streamName 
				 * @param[in] _name 
				 * @return Interfae Pointer.
				 */
				std11::shared_ptr<river::Interface> createInput(float _freq,
				                                                const std::vector<audio::channel>& _map,
				                                                audio::format _format,
				                                                const std::string& _streamName,
				                                                const std::string& _name);
				/**
				 * @brief Stream data input callback
				 * @todo : copy doc ..
				 */
				void onDataReceivedMicrophone(const void* _data,
				                              const std11::chrono::system_clock::time_point& _time,
				                              size_t _nbChunk,
				                              enum audio::format _format,
				                              uint32_t _frequency,
				                              const std::vector<audio::channel>& _map);
				/**
				 * @brief Stream data input callback
				 * @todo : copy doc ..
				 */
				void onDataReceivedFeedBack(const void* _data,
				                            const std11::chrono::system_clock::time_point& _time,
				                            size_t _nbChunk,
				                            enum audio::format _format,
				                            uint32_t _frequency,
				                            const std::vector<audio::channel>& _map);
			protected:
				drain::CircularBuffer m_bufferMicrophone; //!< temporary buffer to synchronize data.
				drain::CircularBuffer m_bufferFeedBack; //!< temporary buffer to synchronize data.
				std11::chrono::nanoseconds m_sampleTime; //!< represent the sample time at the specify frequency.
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
				void processAEC(void* _dataMic, void* _dataFB, uint32_t _nbChunk, const std11::chrono::system_clock::time_point& _time);
			public:
				virtual void generateDot(etk::FSNode& _node);
		};
	}
}

#endif

