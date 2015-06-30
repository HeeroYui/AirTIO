/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <audio/river/widget/debug.h>
#include <audio/river/widget/TemporalViewer.h>
#include <etk/tool.h>

#undef __class__
#define __class__ "TemporalViewer"




audio::river::widget::TemporalViewer::TemporalViewer() :
  m_minVal(-1.0f),
  m_maxVal(1.0f) {
	addObjectType("audio::river::widget::TemporalViewer");
}

void audio::river::widget::TemporalViewer::init() {
	ewol::Widget::init();
	m_manager = audio::river::Manager::create("audio::river::widget::TemporalViewer");
	markToRedraw();
}


audio::river::widget::TemporalViewer::~TemporalViewer() {
	
}



void audio::river::widget::TemporalViewer::onDataReceived(const void* _data,
                                                          const audio::Time& _time,
                                                          size_t _nbChunk,
                                                          enum audio::format _format,
                                                          uint32_t _frequency,
                                                          const std::vector<audio::channel>& _map) {
	std11::unique_lock<std11::mutex> lock(m_mutex);
	if (_format != audio::format_float) {
		std::cout << "[ERROR] call wrong type ... (need int16_t)" << std::endl;
	}
	// get the curent power of the signal.
	const float* data = static_cast<const float*>(_data);
	for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
		m_data.push_back(data[iii]);
	}
	const int32_t nbSecond = 3;
	if (m_data.size()>_frequency*nbSecond) {
		m_data.erase(m_data.begin(), m_data.begin()+(m_data.size()-_frequency*nbSecond));
	}
	//markToRedraw();
}

void audio::river::widget::TemporalViewer::recordToggle() {
	std11::unique_lock<std11::mutex> lock(m_mutex);
	if (m_interface == nullptr) {
		//Get the generic input:
		std::vector<audio::channel> channel;
		channel.push_back(audio::channel_frontLeft);
		m_interface = m_manager->createInput(48000,
		                                     channel,
		                                     audio::format_float,
		                                     "microphone");
		if(m_interface == nullptr) {
			ARW_ERROR("nullptr interface");
			return;
		}
		// set callback mode ...
		m_interface->setInputCallback(std11::bind(&audio::river::widget::TemporalViewer::onDataReceived,
		                                          this,
		                                          std11::placeholders::_1,
		                                          std11::placeholders::_2,
		                                          std11::placeholders::_3,
		                                          std11::placeholders::_4,
		                                          std11::placeholders::_5,
		                                          std11::placeholders::_6));
		// start the stream
		m_interface->start();
		periodicCallEnable();
	} else {
		m_interface->stop();
		m_interface.reset();
		periodicCallDisable();
	}
}

void audio::river::widget::TemporalViewer::onDraw() {
	m_draw.draw();
}


void audio::river::widget::TemporalViewer::onRegenerateDisplay() {
	//!< Check if we really need to redraw the display, if not needed, we redraw the previous data ...
	if (needRedraw() == false) {
		return;
	}
	// remove previous data
	m_draw.clear();
	// set background
	m_draw.setColor(etk::color::black);
	m_draw.setPos(vec2(0,0));
	m_draw.rectangleWidth(m_size);
	std11::unique_lock<std11::mutex> lock(m_mutex);
	if (m_data.size() == 0) {
		return;
	}
	// set all the line:
	m_draw.setColor(etk::color::white);
	m_draw.setThickness(1);
	float origin = m_size.y()*0.5f;
	
	float ratioY = m_size.y() / (m_maxVal - m_minVal);
	float stepX = m_size.x() / float(m_data.size());
	m_draw.setPos(vec2(0, origin + ratioY*m_data[0]));
	float baseX = 0;
	for (size_t iii=1; iii<m_data.size(); ++iii) {
		m_draw.lineTo(vec2(float(iii)*stepX, origin + ratioY*m_data[iii]));
	}
}




void audio::river::widget::TemporalViewer::periodicCall(const ewol::event::Time& _event) {
	markToRedraw();
}
