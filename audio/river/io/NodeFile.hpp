/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#ifdef AUDIO_RIVER_BUILD_FILE

#include <audio/river/io/Node.hpp>
#include <audio/orchestra/Interface.hpp>

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
					NodeFile(const etk::Path& _path, const ejson::Object& _config);
				public:
					static ememory::SharedPtr<NodeFile> create(const etk::Path& _path, const ejson::Object& _config);
					/**
					 * @brief Destructor
					 */
					virtual ~NodeFile();
					virtual bool isHarwareNode() {
						return true;
					};
				protected:
					audio::Time m_time; //!< time of the flow
					ememory::SharedPtr<etk::io::Interface>& m_file; //!< File interface
					bool m_restartAtEnd; //!< The read is done in loop
					uint32_t m_sampleRate; //!< Sample Rate of the Raw file
					audio::format m_format; //!< Format of the file
					etk::Vector<audio::channel> m_map; //!< Map of the file
					ememory::SharedPtr<ethread::Thread> m_thread; //!< playing thread of the flow
					bool m_alive; //!< thread is active
				protected:
					virtual void start();
					virtual void stop();
					virtual void threadCallback():
			};
		}
	}
}
#endif
