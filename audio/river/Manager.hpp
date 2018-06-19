/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <etk/String.hpp>
#include <ememory/memory.hpp>
#include <audio/river/Interface.hpp>
#include <audio/format.hpp>
#include <audio/channel.hpp>
#include <ejson/ejson.hpp>

namespace audio {
	namespace river {
		/**
		 * @brief Audio interface manager : Single interface for every application that want to access on the Audio input/output
		 */
		class Manager  : public ememory::EnableSharedFromThis<Manager> {
			private:
				const etk::String& m_applicationUniqueId; //!< name of the application that open the Audio Interface.
				etk::Vector<ememory::WeakPtr<audio::river::Interface> > m_listOpenInterface; //!< List of all open Stream.
			protected:
				/**
				 * @brief Constructor
				 */
				Manager(const etk::String& _applicationUniqueId);
			public:
				/**
				 * @brief factory of the manager. Every Application will have only one maager for all his flow. this permit to manage all of it
				 * @param[in] _applicationUniqueId Unique name of the application
				 * @return Pointer on the manager or null if an error occured
				 */
				static ememory::SharedPtr<audio::river::Manager> create(const etk::String& _applicationUniqueId);
				/**
				 * @brief Destructor
				 */
				virtual ~Manager();
			public:
				/**
				 * @brief Get all input audio stream.
				 * @return a list of all availlables input stream name
				 */
				etk::Vector<etk::String> getListStreamInput();
				/**
				 * @brief Get all output audio stream.
				 * @return a list of all availlables output stream name
				 */
				etk::Vector<etk::String> getListStreamOutput();
				/**
				 * @brief Get all audio virtual stream.
				 * @return a list of all availlables virtual stream name
				 */
				etk::Vector<etk::String> getListStreamVirtual();
				/**
				 * @brief Get all audio stream.
				 * @return a list of all availlables stream name
				 */
				etk::Vector<etk::String> getListStream();
				
				/**
				 * @brief Set a volume for a specific group
				 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
				 * @param[in] _value Volume in dB to set.
				 * @return true set done
				 * @return false An error occured
				 * @example : setVolume("MASTER", -3.0f);
				 */
				virtual bool setVolume(const etk::String& _volumeName, float _valuedB);
				/**
				 * @brief Get a volume value
				 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
				 * @return The Volume value in dB.
				 * @example ret = getVolume("MASTER"); can return something like ret = -3.0f
				 */
				virtual float getVolume(const etk::String& _volumeName) const;
				
				/**
				 * @brief Get a parameter value
				 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
				 * @return The requested value Range.
				 * @example ret = getVolumeRange("MASTER"); can return something like ret=(-120.0f,0.0f)
				 */
				virtual etk::Pair<float,float> getVolumeRange(const etk::String& _volumeName) const;
				/**
				 * @brief Set a Mute for a specific volume group
				 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
				 * @param[in] _mute Mute enable or disable.
				 */
				virtual void setMute(const etk::String& _volumeName, bool _mute);
				/**
				 * @brief Get a volume value
				 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
				 * @return The Mute of the volume volume.
				 */
				virtual bool getMute(const etk::String& _volumeName) const;
				
				/**
				 * @brief Create output Interface
				 * @param[in] _freq Frequency to open Interface [8,16,22,32,48] kHz
				 * @param[in] _map ChannelMap of the Output
				 * @param[in] _format Sample Format to open the stream [int8_t, int16_t, ...]
				 * @param[in] _streamName Stream name to open: "" or "default" open current selected output
				 * @param[in] _options Json option to configure default resampling and many other things.
				 * @return a pointer on the interface
				 */
				virtual ememory::SharedPtr<Interface> createOutput(float _freq = 48000,
				                                                  const etk::Vector<audio::channel>& _map = etk::Vector<audio::channel>(),
				                                                  audio::format _format = audio::format_int16,
				                                                  const etk::String& _streamName = "",
				                                                  const etk::String& _options = "");
				/**
				 * @brief Create input Interface
				 * @param[in] _freq Frequency to open Interface [8,16,22,32,48] kHz
				 * @param[in] _map ChannelMap of the Output
				 * @param[in] _format Sample Format to open the stream [int8_t, int16_t, ...]
				 * @param[in] _streamName Stream name to open: "" or "default" open current selected input
				 * @param[in] _options Json option to configure default resampling and many other things.
				 * @return a pointer on the interface
				 */
				virtual ememory::SharedPtr<Interface> createInput(float _freq = 48000,
				                                                 const etk::Vector<audio::channel>& _map = etk::Vector<audio::channel>(),
				                                                 audio::format _format = audio::format_int16,
				                                                 const etk::String& _streamName = "",
				                                                 const etk::String& _options = "");
				/**
				 * @brief Create input Feedback Interface
				 * @param[in] _freq Frequency to open Interface [8,16,22,32,48] kHz
				 * @param[in] _map ChannelMap of the Output
				 * @param[in] _format Sample Format to open the stream [int8_t, int16_t, ...]
				 * @param[in] _streamName Stream name to open: "" or "default" open current selected input
				 * @param[in] _options Json option to configure default resampling and many other things.
				 * @return a pointer on the interface
				 */
				virtual ememory::SharedPtr<Interface> createFeedback(float _freq = 48000,
				                                                    const etk::Vector<audio::channel>& _map = etk::Vector<audio::channel>(),
				                                                    audio::format _format = audio::format_int16,
				                                                    const etk::String& _streamName = "",
				                                                    const etk::String& _options = "");
				/**
				 * @brief Generate the dot file corresponding at all the actif nodes.
				 * @param[in] _filename Name of the file to write data.
				 */
				virtual void generateDotAll(const etk::String& _filename);
		};
	}
}

