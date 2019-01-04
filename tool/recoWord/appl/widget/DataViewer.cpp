/** @file
 * @author Edouard DUPIN 
 * @copyright 2011, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <appl/debug.hpp>
#include <appl/widget/DataViewer.hpp>
#include <etk/tool.hpp>
#include <ewol/object/Manager.hpp>

static const int32_t nbSecondOffset = 3;
static const int32_t nbSecondSilence = 1;
static const int32_t nbSecond = 10;
static const int32_t nbSecondPreviousPost = 1;
static const float startThresholdLevel = 0.2;
static const float stopThresholdLevel = 0.1;



appl::widget::DataViewer::DataViewer() :
  m_minVal(-1.0f),
  m_maxVal(1.0f),
  m_sampleRate(48000) {
	addObjectType("appl::widget::DataViewer");
}

void appl::widget::DataViewer::init() {
	ewol::Widget::init();
	m_manager = audio::river::Manager::create("appl::widget::DataViewer");
	m_data.resize(m_sampleRate*nbSecond*3, 0.0);
	m_data.clear();
}


appl::widget::DataViewer::~DataViewer() {
	
}



void appl::widget::DataViewer::onDataReceived(const void* _data,
                                              const audio::Time& _time,
                                              size_t _nbChunk,
                                              enum audio::format _format,
                                              uint32_t _frequency,
                                              const etk::Vector<audio::channel>& _map) {
	ethread::UniqueLock lock(m_mutex);
	if (_format != audio::format_float) {
		APPL_ERROR("call wrong type ... (need int16_t)");
	}
	// get the curent power of the signal.
	const float* data = static_cast<const float*>(_data);
	for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
		m_data.pushBack(data[iii]);
		if (m_startAnalyse == false) {
			if (data[iii] > startThresholdLevel) {
				m_startAnalyse = true;
				m_silenceCount = 0;
				m_detectStartPosition = m_data.size();
				m_detectStopPosition = m_data.size() + 1;
			}
		} else {
			if (data[iii] > stopThresholdLevel) {
				m_silenceCount = 0;
				m_detectStopPosition = m_data.size();
				m_detectMax = etk::max(m_detectMax, etk::abs(data[iii]));
			} else {
				m_silenceCount++;
			}
		}
	}
	/*
	if (m_data.size()>m_sampleRate*nbSecond*10) {
		m_data.erase(m_data.begin(), m_data.begin()+(m_data.size()-m_sampleRate*nbSecond));
	}
	*/
	//markToRedraw();
}

void appl::widget::DataViewer::recordToggle() {
	ethread::UniqueLock lock(m_mutex);
	if (m_interface == null) {
		//Get the generic input:
		etk::Vector<audio::channel> channel;
		channel.pushBack(audio::channel_frontLeft);
		m_interface = m_manager->createInput(m_sampleRate,
		                                     channel,
		                                     audio::format_float,
		                                     "microphone");
		if(m_interface == null) {
			APPL_ERROR("null interface");
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
		m_PCH = getObjectManager().periodicCall.connect(this, &appl::widget::DataViewer::periodicCall);
	} else {
		m_interface->stop();
		m_interface.reset();
		m_PCH.disconnect();
	}
}

void appl::widget::DataViewer::onDraw() {
	m_draw.draw();
}


void appl::widget::DataViewer::onRegenerateDisplay() {
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
	//APPL_INFO("nbSlot : " << nbSlot << " sizeSlot=" << sizeSlot << " m_size=" << m_size);
	list.resize(nbSlot,0.0f);
	int32_t step = m_sampleRate*nbSecond/nbSlot;
	for (size_t kkk=0; kkk<m_sampleRate*nbSecond; ++kkk) {
		int32_t id = kkk/step;
		if (id < list.size()) {
			if (kkk < m_data.size()-m_startDisplayOffset) {
				list[id] = etk::max(list[id], m_data[m_startDisplayOffset+kkk]);
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
			APPL_ERROR("wrong display position");
		}
	}
	// detect sound
	if (m_detectStopPosition > m_detectStartPosition) {
		m_draw.setColor(etk::color::orange);
		m_draw.setThickness(1);
		float posStart = double(m_detectStartPosition-m_startDisplayOffset)/double(step)*sizeSlot;
		float posStop = double(m_detectStopPosition-m_startDisplayOffset)/double(step)*sizeSlot;
		m_draw.setPos(vec2(posStart, origin - ratioY*m_detectMax));
		//m_draw.rectangle(vec2(posStop, origin + ratioY*m_detectMax));
		//APPL_ERROR("draw position: " << posStart << "  " << posStop);
		
		m_draw.lineTo(vec2(posStart, origin + ratioY*m_detectMax));
		m_draw.lineTo(vec2(posStop, origin + ratioY*m_detectMax));
		m_draw.lineTo(vec2(posStop, origin - ratioY*m_detectMax));
		m_draw.lineTo(vec2(posStart, origin - ratioY*m_detectMax));
	}
	// sound recorded
	if (m_detectStopPosition > m_detectStartPosition) {
		m_draw.setColor(etk::color::blue);
		m_draw.setThickness(1);
		float posStart = double(m_detectStartPosition-m_startDisplayOffset-nbSecondPreviousPost*m_sampleRate)/double(step)*sizeSlot;
		float posStop = double(m_detectStopPosition-m_startDisplayOffset+nbSecondPreviousPost*m_sampleRate)/double(step)*sizeSlot;
		m_draw.setPos(vec2(posStart, origin - ratioY*0.5));
		m_draw.lineTo(vec2(posStart, origin + ratioY*0.5));
		m_draw.lineTo(vec2(posStop, origin + ratioY*0.5));
		m_draw.lineTo(vec2(posStop, origin - ratioY*0.5));
		m_draw.lineTo(vec2(posStart, origin - ratioY*0.5));
	}
	// End of silence detection
	if (m_detectStopPosition > m_detectStartPosition) {
		m_draw.setColor(etk::color::red);
		m_draw.setThickness(1);
		float pos = double(m_detectStopPosition-m_startDisplayOffset+nbSecondSilence*m_sampleRate)/double(step)*sizeSlot;
		m_draw.setPos(vec2(pos, origin - ratioY*0.5));
		m_draw.lineTo(vec2(pos, origin + ratioY*0.5));
	}
	if (m_detectStopPosition <= m_detectStartPosition) {
		m_draw.setColor(etk::color::red);
		m_draw.setThickness(1);
		// start threshold
		m_draw.setPos(vec2(0, origin - ratioY*startThresholdLevel));
		m_draw.lineTo(vec2(m_size.x(), origin - ratioY*startThresholdLevel));
		m_draw.setPos(vec2(0, origin + ratioY*startThresholdLevel));
		m_draw.lineTo(vec2(m_size.x(), origin + ratioY*startThresholdLevel));
	} else {
		m_draw.setColor(etk::color::red);
		m_draw.setThickness(1);
		float posStop = double(m_detectStopPosition-m_startDisplayOffset)/double(step)*sizeSlot;
		// start threshold
		m_draw.setPos(vec2(0, origin - ratioY*startThresholdLevel));
		m_draw.lineTo(vec2(posStop, origin - ratioY*startThresholdLevel));
		m_draw.setPos(vec2(0, origin + ratioY*startThresholdLevel));
		m_draw.lineTo(vec2(posStop, origin + ratioY*startThresholdLevel));
		
		// stop threshold
		m_draw.setPos(vec2(posStop, origin - ratioY*stopThresholdLevel));
		m_draw.lineTo(vec2(m_size.x(), origin - ratioY*stopThresholdLevel));
		m_draw.setPos(vec2(posStop, origin + ratioY*stopThresholdLevel));
		m_draw.lineTo(vec2(m_size.x(), origin + ratioY*stopThresholdLevel));
	}
}




void appl::widget::DataViewer::periodicCall(const ewol::event::Time& _event) {
	ethread::UniqueLock lock(m_mutex);
	int32_t nbSampleDelta = _event.getDeltaCall() * float(m_sampleRate);
	if (m_startAnalyse == false) {
		if (m_data.size()>m_sampleRate*nbSecondOffset) {
			m_startDisplayOffset = m_data.size() - m_sampleRate*nbSecondOffset;
			/*
			if (nbSampleDelta < m_data.size()) {
				m_data.erase(m_data.begin(), m_data.begin()+nbSampleDelta);
			} else {
				m_data.erase(m_data.begin(), m_data.begin()+(m_data.size()-m_sampleRate*nbSecond));
			}
			*/
		}
	} else {
		if (m_silenceCount > m_sampleRate*nbSecondSilence) {
			if (m_interface != null) {
				m_interface->stop();
				m_interface.reset();
				m_PCH.disconnect();
			}
		}
	}
	markToRedraw();
}


void appl::widget::DataViewer::reset() {
	ethread::UniqueLock lock(m_mutex);
	m_data.clear();
	m_startDisplayOffset = 0;
	m_startAnalyse = false;
	m_silenceCount = 0;
	m_detectStartPosition = 0;
	m_detectStopPosition = 0;
	m_detectMax = 0;
}


