/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/String.hpp>
#include <etk/Vector.hpp>
#include <cstdint>
#include <ethread/Mutex.hpp>
#include <chrono>
#include <functional>
#include <ememory/memory.hpp>
#include <audio/format.hpp>
#include <audio/channel.hpp>
#include <audio/drain/Process.hpp>
#include <audio/drain/EndPointCallback.hpp>
#include <audio/drain/EndPointWrite.hpp>
#include <ejson/ejson.hpp>
#include <etk/os/FSNode.hpp>
#include <audio/Time.hpp>

namespace audio {
	namespace river {
		namespace io {
			class Node;
			class NodeAirTAudio;
			class NodeAEC;
			class NodeMuxer;
		}
		enum modeInterface {
			modeInterface_unknow,
			modeInterface_input,
			modeInterface_output,
			modeInterface_feedback,
		};
		/**
		 * @brief Interface is the basic handle to manage the input output stream
		 * @note To create this class see @ref audio::river::Manager class
		 */
		class Interface : public ememory::EnableSharedFromThis<Interface> {
			friend class io::Node;
			friend class io::NodeAirTAudio;
			friend class io::NodeAEC;
			friend class io::NodeMuxer;
			friend class Manager;
			protected:
				uint32_t m_uid; //!< unique ID for interface
			protected:
				/**
				 * @brief Constructor (use factory)
				 */
				Interface();
				/**
				 * @brief Initilize the Class (do all that is not manage by constructor) Call by factory.
				 * @param[in] _freq Frequency.
				 * @param[in] _map Channel map organization.
				 * @param[in] _format Sample format
				 * @param[in] _node Low level interface to connect the flow.
				 * @param[in] _config Special configuration of this interface.
				 * @return true Configuration done corectly.
				 * @return false the configuration has an error.
				 */
				bool init(float _freq,
				          const etk::Vector<audio::channel>& _map,
				          audio::format _format,
				          ememory::SharedPtr<audio::river::io::Node> _node,
				          const ejson::Object& _config);
				/**
				 * @brief Factory of this interface (called by class audio::river::Manager)
				 * @param[in] _freq Frequency.
				 * @param[in] _map Channel map organization.
				 * @param[in] _format Sample format
				 * @param[in] _node Low level interface to connect the flow.
				 * @param[in] _config Special configuration of this interface.
				 * @return nullptr The configuration does not work.
				 * @return pointer The interface has been corectly created.
				 */
				static ememory::SharedPtr<Interface> create(float _freq,
				                                           const etk::Vector<audio::channel>& _map,
				                                           audio::format _format,
				                                           const ememory::SharedPtr<audio::river::io::Node>& _node,
				                                           const ejson::Object& _config);
			public:
				/**
				 * @brief Destructor
				 */
				virtual ~Interface();
			protected:
				mutable std::recursive_mutex m_mutex; //!< Local mutex to protect data
				ejson::Object m_config; //!< configuration set by the user.
			protected:
				enum modeInterface m_mode; //!< interface type (input/output/feedback)
			public:
				/**
				 * @brief Get mode type of the current interface.
				 * @return The mode requested.
				 */
				enum modeInterface getMode() {
					return m_mode;
				}
			protected:
				audio::drain::Process m_process; //!< Algorithme processing engine
			public:
				/**
				 * @brief Get the interface format configuration.
				 * @return The current format.
				 */
				const audio::drain::IOFormatInterface& getInterfaceFormat() {
					if (    m_mode == modeInterface_input
					     || m_mode == modeInterface_feedback) {
						return m_process.getOutputConfig();
					} else {
						return m_process.getInputConfig();
					}
				}
			protected:
				ememory::SharedPtr<audio::river::io::Node> m_node; //!< Hardware interface to/from stream audio flow.
			protected:
				etk::String m_name; //!< Name of the interface.
			public:
				/**
				 * @brief Get interface name.
				 * @return The current name.
				 */
				virtual etk::String getName() {
					return m_name;
				};
				/**
				 * @brief Set the interface name
				 * @param[in] _name new name of the interface
				 */
				virtual void setName(const etk::String& _name) {
					m_name = _name;
				};
				/**
				 * @brief set the read/write mode enable.
				 * @note If you not set a output/input callback you must call this function.
				 */
				virtual void setReadwrite();
				/**
				 * @brief When we want to implement a Callback Mode:
				 */
				/**
				 * @brief Set a callback on the write mode interface to know when data is needed in the buffer
				 * @param[in] _function Function to call
				 */
				virtual void setWriteCallback(audio::drain::playbackFunctionWrite _function);
				/**
				 * @brief Set Output callback mode with the specify callback.
				 * @param[in] _function Function to call
				 */
				virtual void setOutputCallback(audio::drain::playbackFunction _function);
				/**
				 * @brief Set Input callback mode with the specify callback.
				 * @param[in] _function Function to call
				 */
				virtual void setInputCallback(audio::drain::recordFunction _function);
				/**
				 * @brief Add a volume group of the current channel.
				 * @note If you do not call this function with the group "FLOW" you chan not have a channel volume.
				 * @note the set volume stage can not be set after the start.
				 * @param[in] _name Name of the group classicle common group:
				 *                      - FLOW for channel volume.
				 *                      - MEDIA for multimedia volume control (audio player, player video, web streaming ...).
				 *                      - TTS for Test-to-speech volume control.
				 *                      - COMMUNICATION for user communication volume control.
				 *                      - NOTIFICATION for urgent notification volume control.
				 *                      - NOISE for small noise volume control.
				 */
				virtual void addVolumeGroup(const etk::String& _name);
			public:
				/**
				 * @brief Start the Audio interface flow.
				 * @param[in] _time Time to start the flow (0) to start as fast as possible...
				 * @note _time to play buffer when output interface (if possible)
				 * @note _time to read buffer when inut interface (if possible)
				 */
				virtual void start(const audio::Time& _time = audio::Time());
				/**
				 * @brief Stop the current flow.
				 * @param[in] _fast The stream stop as fast as possible (not write all the buffer in speaker) but apply cross fade out.
				 * @param[in] _abort The stream stop whith no garenty of good audio stop.
				 */
				virtual void stop(bool _fast=false, bool _abort=false);
				/**
				 * @brief Abort flow (no audio garenty)
				 */
				virtual void abort();
				/**
				 * @brief Set a parameter in the stream flow
				 * @param[in] _filter name of the filter (if you added some personels)
				 * @param[in] _parameter Parameter name.
				 * @param[in] _value Value to set.
				 * @return true set done
				 * @return false An error occured
				 * @example : setParameter("volume", "FLOW", "-3dB");
				 * @example : setParameter("LowPassFilter", "cutFrequency", "1000Hz");
				 */
				virtual bool setParameter(const etk::String& _filter, const etk::String& _parameter, const std::string& _value);
				/**
				 * @brief Get a parameter value
				 * @param[in] _filter name of the filter (if you added some personels)
				 * @param[in] _parameter Parameter name.
				 * @return The requested value.
				 * @example : getParameter("volume", "FLOW"); can return something like "-3dB"
				 * @example : getParameter("LowPassFilter", "cutFrequency"); can return something like "[-120..0]dB"
				 */
				virtual etk::String getParameter(const etk::String& _filter, const std::string& _parameter) const;
				/**
				 * @brief Get a parameter value
				 * @param[in] _filter name of the filter (if you added some personels)
				 * @param[in] _parameter Parameter name.
				 * @return The requested value.
				 * @example : getParameter("volume", "FLOW"); can return something like "[-120..0]dB"
				 * @example : getParameter("LowPassFilter", "cutFreqiency"); can return something like "]100..10000]Hz"
				 */
				virtual etk::String getParameterProperty(const etk::String& _filter, const std::string& _parameter) const;
				/**
				 * @brief write some audio sample in the speakers
				 * @param[in] _value Data To write on output
				 * @param[in] _nbChunk Number of audio chunk to write
				 */
				// TODO : TimeOut ???
				virtual void write(const void* _value, size_t _nbChunk);
				/**
				 * @brief read some audio sample from Microphone
				 * @param[in] _value Data To write on output
				 * @param[in] _nbChunk Number of audio chunk to write
				 */
				// TODO : TimeOut ???
				virtual void read(void* _value, size_t _nbChunk);
				/**
				 * @brief Get number of chunk in the local buffer
				 * @return Number of chunk
				 */
				virtual size_t size() const;
				/**
				 * @brief Set buffer size in chunk number
				 * @param[in] _nbChunk Number of chunk in the buffer
				 */
				virtual void setBufferSize(size_t _nbChunk);
				/**
				 * @brief Set buffer size size of the buffer with the stored time in µs
				 * @param[in] _time Time in microsecond of the buffer
				 */
				virtual void setBufferSize(const std::chrono::microseconds& _time);
				/**
				 * @brief get buffer size in chunk number
				 * @return Number of chunk that can be written in the buffer
				 */
				virtual size_t getBufferSize();
				/**
				 * @brief Set buffer size size of the buffer with the stored time in µs
				 * @return Time in microsecond that can be written in the buffer
				 */
				virtual std::chrono::microseconds getBufferSizeMicrosecond();
				/**
				 * @brief Get buffer size filled in chunk number
				 * @return Number of chunk in the buffer (that might be read/write)
				 */
				virtual size_t getBufferFillSize();
				/**
				 * @brief Set buffer size size of the buffer with the stored time in µs
				 * @return Time in microsecond of the buffer (that might be read/write)
				 */
				virtual std::chrono::microseconds getBufferFillSizeMicrosecond();
				/**
				 * @brief Remove internal Buffer
				 */
				virtual void clearInternalBuffer();
				/**
				 * @brief Write : Get the time of the next sample time to write in the local buffer
				 * @brief Read : Get the time of the next sample time to read in the local buffer
				 */
				virtual audio::Time getCurrentTime() const;
			private:
				/**
				 * @brief Node Call interface : Input interface node has new data.
				 * @param[in] _time Time where the first sample has been capture.
				 * @param[in] _data Pointer on the new data.
				 * @param[in] _nbChunk Number of chunk in the buffer.
				 */
				virtual void systemNewInputData(audio::Time _time, const void* _data, size_t _nbChunk);
				/**
				 * @brief Node Call interface: Output interface node need new data.
				 * @param[in] _time Time where the data might be played
				 * @param[in] _data Pointer on the data.
				 * @param[in] _nbChunk Number of chunk that might be write
				 * @param[in] _chunkSize Chunk size.
				 */
				virtual void systemNeedOutputData(audio::Time _time, void* _data, size_t _nbChunk, size_t _chunkSize);
				/**
				 * @brief Node Call interface: A volume has change.
				 */
				virtual void systemVolumeChange();
			public:
				/**
				 * @brief Create the dot in the FileNode stream.
				 * @param[in,out] _node File node to write data.
				 * @param[in] _nameIO Name to link the interface node
				 * @param[in] _isLink True if the node is connected on the current interface.
				 */
				virtual void generateDot(etk::FSNode& _node, const etk::String& _nameIO, bool _isLink=true);
				/**
				 * @brief Get the current 'dot' name of the interface
				 * @return The anme requested.
				 */
				virtual etk::String getDotNodeName() const;
			protected:
				/**
				 * @brief Interfanel generate of status
				 * @param[in] _origin status source
				 * @param[in] _status Event status
				 */
				void generateStatus(const etk::String& _origin, const etk::String& _status) {
					m_process.generateStatus(_origin, _status);
				}
			public:
				/**
				 * @brief Set status callback
				 * @param[in] _newFunction Function to call
				 */
				void setStatusFunction(audio::drain::statusFunction _newFunction) {
					m_process.setStatusFunction(_newFunction);
				}
		};
	}
}
