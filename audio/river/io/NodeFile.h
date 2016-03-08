/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#ifdef AUDIO_RIVER_BUILD_FILE

#include <audio/river/io/Node.h>
#include <audio/orchestra/Interface.h>

namespace audio {
	namespace river {
		namespace io {
			class Manager;
			class Group;
			/**
			 * @brief Low level node that is manage on the interface with the extern lib airtaudio
			 */
			class NodeFile : public audio::river::io::Node {
				friend class audio::river::io::Group;
				protected:
					/**
					 * @brief Constructor
					 */
					NodeFile(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
				public:
					static std::shared_ptr<NodeFile> create(const std::string& _name, const std::shared_ptr<const ejson::Object>& _config);
					/**
					 * @brief Destructor
					 */
					virtual ~NodeFile();
					virtual bool isHarwareNode() {
						return true;
					};
				protected:
					audio::Time m_time; //!< time of the flow
					etk::FSNode m_file; //!< File interface
					bool m_restartAtEnd; //!< The read is done in loop
					uint32_t m_sampleRate; //!< Sample Rate of the Raw file
					audio::format m_format; //!< Format of the file
					std::vector<audio::channel> m_map; //!< Map of the file
					std::shared_ptr<std::thread> m_thread; //!< playing thread of the flow
					std::atomic<bool> m_alive; //!< thread is active
				protected:
					virtual void start();
					virtual void stop();
					virtual void threadCallback():
			};
		}
	}
}
#endif
