/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <audio/river/widget/debug.hpp>
#include <audio/river/widget/TemporalViewer.hpp>
#include <etk/tool.hpp>
#include <ewol/object/Manager.hpp>

static const int32_t nbSecond = 3;



audio::river::widget::TemporalViewer::TemporalViewer() :
  m_minVal(-1.0f),
  m_maxVal(1.0f),
  m_sampleRate(48000) {
	addObjectType("audio::river::widget::TemporalViewer");
}

void audio::river::widget::TemporalViewer::init() {
	ewol::Widget::init();
	m_manager = audio::river::Manager::create("audio::river::widget::TemporalViewer");
	m_data.resize(m_sampleRate*3, 0.0);
}


audio::river::widget::TemporalViewer::~TemporalViewer() {
	
}



void audio::river::widget::TemporalViewer::onDataReceived(const void* _data,
                                                          const audio::Time& _time,
                                                          size_t _nbChunk,
                                                          enum audio::format _format,
                                                          uint32_t _frequency,
                                                          const etk::Vector<audio::channel>& _map) {
	ethread::UniqueLock lock(m_mutex);
	if (_format != audio::format_float) {
		ARW_ERROR("call wrong type ... (need int16_t)");
	}
	// get the curent power of the signal.
	const float* data = static_cast<const float*>(_data);
	for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
		m_data.pushBack(data[iii]);
	}
	/*
	if (m_data.size()>m_sampleRate*nbSecond*10) {
		m_data.erase(m_data.begin(), m_data.begin()+(m_data.size()-m_sampleRate*nbSecond));
	}
	*/
	//markToRedraw();
}

void audio::river::widget::TemporalViewer::recordToggle() {
	ethread::UniqueLock lock(m_mutex);
	if (m_interface == nullptr) {
		//Get the generic input:
		etk::Vector<audio::channel> channel;
		channel.pushBack(audio::channel_frontLeft);
		m_interface = m_manager->createInput(m_sampleRate,
		                                     channel,
		                                     audio::format_float,
		                                     "microphone");
		if(m_interface == nullptr) {
			ARW_ERROR("nullptr interface");
			return;
		}
		// set callback mode ...
		m_interface->setInputCallback([&](const void* _data,
		                                  const audio::Time& _time,
		                                  size_t _nbChunk,
		                                  enum audio::format _format,
		                                  uint32_t _frequency,
		                                  const etk::Vector<audio::channel>& _map) {
		                                  	onDataReceived(_data, _time, _nbChunk, _format, _frequency, _map);
		                                  });
		// start the stream
		m_interface->start();
		m_PCH = getObjectManager().periodicCall.connect(this, &audio::river::widget::TemporalViewer::periodicCall);
	} else {
		m_interface->stop();
		m_interface.reset();
		m_PCH.disconnect();
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
	ethread::UniqueLock lock(m_mutex);
	if (m_data.size() == 0) {
		return;
	}
	// create n section for display:
	int32_t nbSlot = m_size.x();
	int32_t sizeSlot = m_size.x()/nbSlot;
	etk::Vector<float> list;
	//ARW_INFO("nbSlot : " << nbSlot << " sizeSlot=" << sizeSlot << " m_size=" << m_size);
	list.resize(nbSlot,0.0f);
	int32_t step = m_sampleRate*nbSecond/nbSlot;
	for (size_t kkk=0; kkk<m_sampleRate*nbSecond; ++kkk) {
		int32_t id = kkk/step;
		if (id < list.size()) {
			if (kkk < m_data.size()) {
				list[id] = etk::max(list[id],m_data[kkk]);
			}
		}
	}
	// set all the line:
	m_draw.setColor(etk::color::white);
	m_draw.setThickness(1);
	float origin = m_size.y()*0.5f;
	
	float ratioY = m_size.y() / (m_maxVal - m_minVal);
	float baseX = 0;
	for (size_t iii=1; iii<list.size(); ++iii) {
		m_draw.setPos(vec2(iii*sizeSlot, origin - ratioY*list[iii]));
		m_draw.rectangle(vec2((iii+1)*sizeSlot, origin + ratioY*list[iii]));
		if ((iii+1)*sizeSlot > m_size.x()) {
			ARW_ERROR("wrong display position");
		}
	}
}




void audio::river::widget::TemporalViewer::periodicCall(const ewol::event::Time& _event) {
	ethread::UniqueLock lock(m_mutex);
	int32_t nbSampleDelta = _event.getDeltaCall() * float(m_sampleRate);
	if (m_data.size()>m_sampleRate*nbSecond) {
		if (nbSampleDelta < m_data.size()) {
			m_data.erase(m_data.begin(), m_data.begin()+nbSampleDelta);
		} else {
			m_data.erase(m_data.begin(), m_data.begin()+(m_data.size()-m_sampleRate*nbSecond));
		}
	}
	markToRedraw();
}
