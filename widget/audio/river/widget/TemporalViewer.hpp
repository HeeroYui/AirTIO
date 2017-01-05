/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <ewol/widget/Widget.hpp>
#include <ewol/compositing/Drawing.hpp>
#include <audio/river/river.hpp>
#include <audio/river/Manager.hpp>
#include <audio/river/Interface.hpp>
#include <mutex>

namespace audio {
	namespace river {
		namespace widget {
			class TemporalViewer : public ewol::Widget {
				private:
					mutable std::mutex m_mutex;
				private:
					ewol::compositing::Drawing m_draw; //!< drawing instance
				protected:
					//! @brief constructor
					TemporalViewer();
					void init();
				public:
					DECLARE_WIDGET_FACTORY(TemporalViewer, "TemporalViewer");
					//! @brief destructor
					virtual ~TemporalViewer();
					
					void recordToggle();
					void generateToggle() {
						// ...
					}
				private:
					std::vector<float> m_data;
				private:
					float m_minVal; //!< display minimum value
					float m_maxVal; //!< display maximum value
				public: // herited function
					virtual void onDraw();
					virtual void onRegenerateDisplay();
					virtual void periodicCall(const ewol::event::Time& _event);
				private:
					ememory::SharedPtr<audio::river::Manager> m_manager;
					ememory::SharedPtr<audio::river::Interface> m_interface;
					void onDataReceived(const void* _data,
					                    const audio::Time& _time,
					                    size_t _nbChunk,
					                    enum audio::format _format,
					                    uint32_t _frequency,
					                    const std::vector<audio::channel>& _map);
					int32_t m_sampleRate;
			};
		}
	}
}

