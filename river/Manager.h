/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_MANAGER_H__
#define __RIVER_MANAGER_H__

#include <string>
#include <stdint.h>
#if __cplusplus >= 201103L
	#include <memory>
#else
	#include <etk/memory.h>
#endif
#include <river/Interface.h>
#include <audio/format.h>
#include <audio/channel.h>
#include <ejson/ejson.h>

namespace river {
	/**
	 * @brief Audio interface manager : Single interface for every application that want to access on the Audio input/output
	 */
	class Manager {
		private:
			ejson::Document m_config; // virtual configuration
			const std::string& m_applicationUniqueId; //!< name of the application that open the Audio Interface.
			std::vector<std11::weak_ptr<river::Interface> > m_listOpenInterface; //!< List of all open Stream.
		protected:
			/**
			 * @brief Constructor
			 */
			Manager(const std::string& _applicationUniqueId);
		public:
			static std11::shared_ptr<river::Manager> create(const std::string& _applicationUniqueId);
			/**
			 * @brief Destructor
			 */
			virtual ~Manager();
		public:
			/**
			 * @brief Get all input audio stream description.
			 * @return a list of all availlables input stream (name + description)
			 */
			virtual std::vector<std::pair<std::string,std::string> > getListStreamInput();
			/**
			 * @brief Get all output audio stream description.
			 * @return a list of all availlables output stream (name + description)
			 */
			virtual std::vector<std::pair<std::string,std::string> > getListStreamOutput();
			
			/**
			 * @brief Set a volume for a specific group
			 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
			 * @param[in] _value Volume in dB to set.
			 * @return true set done
			 * @return false An error occured
			 * @example : setVolume("MASTER", -3.0f);
			 */
			virtual bool setVolume(const std::string& _volumeName, float _valuedB);
			/**
			 * @brief Get a volume value
			 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
			 * @return The Volume value in dB.
			 * @example ret = getVolume("MASTER"); can return something like ret = -3.0f
			 */
			virtual float getVolume(const std::string& _volumeName) const;
			/**
			 * @brief Get a parameter value
			 * @param[in] _volumeName Name of the volume (MASTER, MATER_BT ...)
			 * @return The requested value Range.
			 * @example ret = getVolumeRange("MASTER"); can return something like ret=(-120.0f,0.0f)
			 */
			virtual std::pair<float,float> getVolumeRange(const std::string& _volumeName) const;
			
			/**
			 * @brief Create output Interface
			 * @param[in] _freq Frequency to open Interface [8,16,22,32,48] kHz
			 * @param[in] _map ChannelMap of the Output
			 * @param[in] _format Sample Format to open the stream [int8_t]
			 * @param[in] _streamName Stream name to open: "" or "default" open current selected output
			 * @param[in] _name Name of this interface
			 * @return a pointer on the interface
			 */
			virtual std11::shared_ptr<Interface> createOutput(float _freq,
			                                                const std::vector<audio::channel>& _map,
			                                                audio::format _format,
			                                                const std::string& _streamName = "",
			                                                const std::string& _name = "");
			/**
			 * @brief Create input Interface
			 * @param[in] _freq Frequency to open Interface [8,16,22,32,48] kHz
			 * @param[in] _map ChannelMap of the Output
			 * @param[in] _format Sample Format to open the stream [int8_t]
			 * @param[in] _streamName Stream name to open: "" or "default" open current selected input
			 * @param[in] _name Name of this interface
			 * @return a pointer on the interface
			 */
			virtual std11::shared_ptr<Interface> createInput(float _freq,
			                                               const std::vector<audio::channel>& _map,
			                                               audio::format _format,
			                                               const std::string& _streamName = "",
			                                               const std::string& _name = "");
			/**
			 * @brief Generate the dot file corresponding at all the actif nodes.
			 * @param[in] _filename Name of the file to write data.
			 */
			virtual void generateDotAll(const std::string& _filename);
	};
};

#endif
