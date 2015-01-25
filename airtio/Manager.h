/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __AIRTIO_MANAGER_H__
#define __AIRTIO_MANAGER_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <airtio/Interface.h>
#include <airtalgo/format.h>
#include <airtalgo/channel.h>

namespace airtio {
	/**
	 * @brief Audio interface manager : Single interface for every application that want to access on the Audio input/output
	 */
	class Manager {
		private:
			const std::string& m_applicationUniqueId; //!< name of the application that open the Audio Interface.
			std::list<std::weak_ptr<airtio::InterfaceDirect> > m_listOpenInterface; //!< List of all open Stream.
		protected:
			/**
			 * @brief Constructor
			 */
			Manager() {};
		public:
			static std::shared_ptr<airtio::Manager> create(const std::string& _applicationUniqueId);
			/**
			 * @brief Destructor
			 */
			virtual ~Manager() {};
		public:
			/**
			 * @brief Get all input audio stream description.
			 * @return a list of all availlables input stream (name + description)
			 */
			virtual std::vector<std::pair<std::string,std::string> > getListStreamInput() = 0;
			/**
			 * @brief Get all output audio stream description.
			 * @return a list of all availlables output stream (name + description)
			 */
			virtual std::vector<std::pair<std::string,std::string> > getListStreamOutput() = 0;
		protected:
			float m_masterVolume;
			std::pair<float,float> m_masterVolumeRange;
		public:
			/**
			 * @brief Set the output volume master of the Audio interface
			 * @param[in] _gainDB Gain in decibel to apply in volume Master
			 */
			virtual void setMasterOutputVolume(float _gainDB);
			/**
			 * @brief Get the output volume master of the Audio interface
			 * @return The gain in decibel applyied in volume Master
			 */
			virtual float getMasterOutputVolume();
			/**
			 * @brief Get the output volume master range of the Audio interface
			 * @return The gain in decibel range of the output volume Master
			 */
			virtual std::pair<float,float> getMasterOutputVolumeRange();
			
			/**
			 * @brief Set the section volume of the Audio interface
			 * @param[in] _gainDB Gain in decibel to apply in volume section
			 * @param[in] _section section name to apply volume (a section is : tts, reco, player, interjection ...)
			 */
			virtual void setSectionVolume(const std::string& _section, float _gainDB);
			/**
			 * @brief Get the section volume of the Audio interface
			 * @param[in] _section section name to apply volume (a section is : tts, reco, player, interjection ...)
			 * @return The gain in decibel applyied in volume section
			 */
			virtual float getSectionVolume(const std::string& _section);
			/**
			 * @brief Get the section volume range of the Audio interface
			 * @param[in] _section section name to apply volume (a section is : tts, reco, player, interjection ...)
			 * @return The gain in decibel range of the section volume
			 */
			virtual std::pair<float,float> getSectionVolumeRange(const std::string& _section);
			
			/**
			 * @brief Create output Interface
			 * @param[in] _freq Frequency to open Interface [8,16,22,32,48] kHz
			 * @param[in] _map ChannelMap of the Output
			 * @param[in] _format Sample Format to open the stream [int8_t]
			 * @param[in] _streamName Stream name to open: "" or "default" open current selected output
			 * @param[in] _name Name of this interface
			 * @return a pointer on the interface
			 */
			virtual std::shared_ptr<Interface> createOutput(float _freq,
			                                                const std::vector<airtalgo::channel>& _map,
			                                                airtalgo::format _format,
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
			virtual std::shared_ptr<Interface> createInput(float _freq,
			                                               const std::vector<airtalgo::channel>& _map,
			                                               airtalgo::format _format,
			                                               const std::string& _streamName = "",
			                                               const std::string& _name = "");
	};
};

#endif
