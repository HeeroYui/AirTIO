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
#include <ethread/Mutex.hpp>

namespace appl {
	namespace widget {
		class DataViewer : public ewol::Widget {
			private:
				mutable ethread::Mutex m_mutex;
			private:
				ewol::compositing::Drawing m_draw; //!< drawing instance
			protected:
				//! @brief constructor
				DataViewer();
				void init();
			public:
				DECLARE_WIDGET_FACTORY(DataViewer, "DataViewer");
				//! @brief destructor
				virtual ~DataViewer();
				
				void recordToggle();
				void reset();
			private:
				etk::Vector<float> m_data;
				size_t m_startDisplayOffset = 0;
				bool m_startAnalyse = false;
				size_t m_silenceCount = 0;
				float m_detectMax = 0;
				size_t m_detectStartPosition = 0;
				size_t m_detectStopPosition = 0;
			private:
				float m_minVal; //!< display minimum value
				float m_maxVal; //!< display maximum value
			public: // herited function
				virtual void onDraw();
				virtual void onRegenerateDisplay();
			protected:
				esignal::Connection m_PCH; //!< Periodic Call Handle to remove it when needed
				/**
				 * @brief Periodic call to update grapgic display
				 * @param[in] _event Time generic event
				 */
				virtual void periodicCall(const ewol::event::Time& _event);
			private:
				ememory::SharedPtr<audio::river::Manager> m_manager;
				ememory::SharedPtr<audio::river::Interface> m_interface;
				void onDataReceived(const void* _data,
				                    const audio::Time& _time,
				                    size_t _nbChunk,
				                    enum audio::format _format,
				                    uint32_t _frequency,
				                    const etk::Vector<audio::channel>& _map);
				int32_t m_sampleRate;
		};
	}
}

