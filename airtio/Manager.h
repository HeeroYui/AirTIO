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
			std::vector<std::weak_ptr<airtio::Interface> > m_listOpenInterface; //!< List of all open Stream.
		protected:
			/**
			 * @brief Constructor
			 */
			Manager(const std::string& _applicationUniqueId);
		public:
			static std::shared_ptr<airtio::Manager> create(const std::string& _applicationUniqueId);
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
			 * @brief Set a parameter in the stream flow
			 * @param[in] _flow Low level Flow name (see json config file)
			 * @param[in] _filter name of the filter (if you added some personels)
			 * @param[in] _parameter Parameter name.
			 * @param[in] _value Value to set.
			 * @return true set done
			 * @return false An error occured
			 * @example : setParameter("speaker", "volume", "MASTER", "-3dB");
			 * @example : setParameter("microphone", "LowPassFilter", "cutFrequency", "1000Hz");
			 */
			virtual bool setParameter(const std::string& _flow, const std::string& _filter, const std::string& _parameter, const std::string& _value);
			/**
			 * @brief Get a parameter value
			 * @param[in] _flow Low level Flow name (see json config file)
			 * @param[in] _filter name of the filter (if you added some personels)
			 * @param[in] _parameter Parameter name.
			 * @return The requested value.
			 * @example : getParameter("speaker", "volume", "MASTER"); can return something like "-3dB"
			 * @example : getParameter("microphone", "LowPassFilter", "cutFrequency"); can return something like "[-120..0]dB"
			 */
			virtual std::string getParameter(const std::string& _flow, const std::string& _filter, const std::string& _parameter) const;
			/**
			 * @brief Get a parameter value
			 * @param[in] _flow Low level Flow name (see json config file)
			 * @param[in] _filter name of the filter (if you added some personels)
			 * @param[in] _parameter Parameter name.
			 * @return The requested value.
			 * @example : getParameter("speaker", "volume", "MASTER"); can return something like "[-120..0]dB"
			 * @example : getParameter("microphone", "LowPassFilter", "cutFreqiency"); can return something like "]100..10000]Hz"
			 */
			virtual std::string getParameterProperty(const std::string& _flow, const std::string& _filter, const std::string& _parameter) const;
			
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
