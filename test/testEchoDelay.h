/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_TEST_ECHO_DELAY_H__
#define __RIVER_TEST_ECHO_DELAY_H__

#include <river/debug.h>

#undef __class__
#define __class__ "test_echo_delay"

namespace river_test_echo_delay {
	class TestClass {
		private:
			std11::shared_ptr<river::Manager> m_manager;
			std11::shared_ptr<river::Interface> m_interfaceOut;
			std11::shared_ptr<river::Interface> m_interfaceIn;
			std11::shared_ptr<river::Interface> m_interfaceFB;
			double m_phase;
			double m_freq;
			int32_t m_nextSampleCount;
			std11::chrono::milliseconds m_delayBetweenEvent;
			std11::chrono::system_clock::time_point m_nextTick;
			std11::chrono::system_clock::time_point m_currentTick;
			int32_t m_stateFB;
			int32_t m_stateMic;
		public:
			TestClass(std11::shared_ptr<river::Manager> _manager) :
			  m_manager(_manager),
			  m_phase(0),
			  m_freq(400),
			  m_nextSampleCount(0),
			  m_delayBetweenEvent(400),
			  m_stateFB(3),
			  m_stateMic(3) {
				//Set stereo output:
				std::vector<audio::channel> channelMap;
				channelMap.push_back(audio::channel_frontLeft);
				channelMap.push_back(audio::channel_frontRight);
				m_interfaceOut = m_manager->createOutput(48000,
				                                         channelMap,
				                                         audio::format_int16,
				                                         "speaker",
				                                         "delayTestOut");
				// set callback mode ...
				m_interfaceOut->setOutputCallback(std11::bind(&TestClass::onDataNeeded,
				                                              this,
				                                              std11::placeholders::_1,
				                                              std11::placeholders::_2,
				                                              std11::placeholders::_3,
				                                              std11::placeholders::_4,
				                                              std11::placeholders::_5,
				                                              std11::placeholders::_6));
				m_interfaceOut->addVolumeGroup("FLOW");
				
				m_interfaceIn = m_manager->createInput(48000,
				                                       channelMap,
				                                       audio::format_int16,
				                                       "microphone",
				                                       "delayTestIn");
				// set callback mode ...
				m_interfaceIn->setInputCallback(std11::bind(&TestClass::onDataReceived,
				                                            this,
				                                            std11::placeholders::_1,
				                                            std11::placeholders::_2,
				                                            std11::placeholders::_3,
				                                            std11::placeholders::_4,
				                                            std11::placeholders::_5,
				                                            std11::placeholders::_6));
				
				m_interfaceFB = m_manager->createInput(48000,
				                                       channelMap,
				                                       audio::format_int16,
				                                       "feedback",
				                                       "delayTestFB");
				// set callback mode ...
				m_interfaceFB->setInputCallback(std11::bind(&TestClass::onDataReceivedFeedBack,
				                                            this,
				                                            std11::placeholders::_1,
				                                            std11::placeholders::_2,
				                                            std11::placeholders::_3,
				                                            std11::placeholders::_4,
				                                            std11::placeholders::_5,
				                                            std11::placeholders::_6));
				
				m_manager->generateDotAll("activeProcess.dot");
			}
			void onDataNeeded(void* _data,
			                  const std11::chrono::system_clock::time_point& _time,
			                  size_t _nbChunk,
			                  enum audio::format _format,
			                  uint32_t _frequency,
			                  const std::vector<audio::channel>& _map) {
				int16_t* data = static_cast<int16_t*>(_data);
				double baseCycle = 2.0*M_PI/(double)48000 * m_freq;
				if (_time == std11::chrono::system_clock::time_point()) {
					for (int32_t iii=0; iii<_nbChunk; iii++) {
						for (int32_t jjj=0; jjj<_map.size(); jjj++) {
							data[_map.size()*iii+jjj] = 0;
						}
					}
					return;
				}
				if (m_nextTick == std11::chrono::system_clock::time_point()) {
					m_nextTick = _time + m_delayBetweenEvent;
					m_nextSampleCount = m_delayBetweenEvent.count()*int64_t(_frequency)/1000;
					m_phase = -1;
				}
				//APPL_INFO("sample : " << m_nextSampleCount);
				for (int32_t iii=0; iii<_nbChunk; iii++) {
					if (m_nextSampleCount > 0) {
						m_nextSampleCount--;
					} else {
						m_phase = 0;
						m_nextSampleCount = m_delayBetweenEvent.count()*int64_t(_frequency)/1000;
						m_currentTick = m_nextTick;
						m_nextTick += m_delayBetweenEvent;
					}
					if (m_phase >= 0) {
						for (int32_t jjj=0; jjj<_map.size(); jjj++) {
							data[_map.size()*iii+jjj] = sin(m_phase) * 30000;
						}
						double newPhase = m_phase+baseCycle;
						if (    m_phase < M_PI
						     && newPhase >= M_PI) {
							// the zero crossing position :
							m_currentTick = getInterpolateTime(_time, iii, sin(m_phase) * 30000, sin(newPhase) * 30000, _frequency);
							// start detection ...
							m_stateFB = 0;
							m_stateMic = 0;
							APPL_WARNING("Time Pulse zero crossing: " << m_currentTick << " id=" << iii);
						}
						m_phase = newPhase;
						if (m_phase >= 2*M_PI) {
							m_phase = -1;
							//m_freq += 50.0;
							if (m_freq>20000.0) {
								m_freq = 400.0;
							}
						}
					} else {
						for (int32_t jjj=0; jjj<_map.size(); jjj++) {
							data[_map.size()*iii+jjj] = 0;
						}
					}
				}
			}
			std11::chrono::system_clock::time_point getInterpolateTime(std11::chrono::system_clock::time_point _time, int32_t _pos, int16_t _val1, int16_t _val2, uint32_t _frequency) {
				if (_val1 == 0) {
					return _time + std11::chrono::nanoseconds(int64_t(_pos)*1000000000LL/int64_t(_frequency));
				} else if (_val2 == 0) {
					return _time + std11::chrono::nanoseconds(int64_t(_pos+1)*1000000000LL/int64_t(_frequency));
				}
				double xxx = double(-_val1) / double(_val2 - _val1);
				APPL_VERBOSE("deltaPos:" << xxx);
				return _time + std11::chrono::nanoseconds(int64_t((double(_pos)+xxx)*1000000000.0)/int64_t(_frequency));
			}
			
			void onDataReceivedFeedBack(const void* _data,
			                            const std11::chrono::system_clock::time_point& _time,
			                            size_t _nbChunk,
			                            enum audio::format _format,
			                            uint32_t _frequency,
			                            const std::vector<audio::channel>& _map) {
				if (_format != audio::format_int16) {
					APPL_ERROR("call wrong type ... (need int16_t)");
				}
				RIVER_SAVE_FILE_MACRO(int16_t, "REC_FeedBack.raw", _data, _nbChunk*_map.size());
				const int16_t* data = static_cast<const int16_t*>(_data);
				// Detect Zero crossing after a max/min ...
				for (size_t iii=0; iii<_nbChunk; ++iii) {
					//for (size_t jjj=0; jjj<_map.size(); ++jjj) {
					size_t jjj=0; {
						if (m_stateFB == 0) {
							if (data[iii*_map.size() + jjj] > INT16_MAX/5) {
								m_stateFB = 1;
								APPL_VERBOSE("FB: detect Normal " << iii);
							} else if (data[iii*_map.size() + jjj] < -INT16_MAX/5) {
								m_stateFB = 2;
								APPL_VERBOSE("FB: detect inverse " << iii);
							}
						} else if (m_stateFB == 1) {
							// normale phase
							if (data[iii*_map.size() + jjj] <= 0) {
								// detect inversion of signe ...
								m_stateFB = 3;
								std11::chrono::system_clock::time_point time = getInterpolateTime(_time, iii-1, data[(iii-1)*_map.size() + jjj], data[iii*_map.size() + jjj], _frequency);
								APPL_VERBOSE("FB:  1 position -1:  " << iii-1 << " " << data[(iii-1)*_map.size() + jjj]);
								APPL_VERBOSE("FB:  1 position  0: " << iii << " " << data[iii*_map.size() + jjj]);
								APPL_WARNING("FB:  1 time detected:     " << time << " delay = " << float((time-m_currentTick).count())/1000.0f << "µs");
							}
						} else if (m_stateFB == 2) {
							// inverse phase
							if (data[iii*_map.size() + jjj] >= 0) {
								// detect inversion of signe ...
								m_stateFB = 3;
								std11::chrono::system_clock::time_point time = getInterpolateTime(_time, iii-1, data[(iii-1)*_map.size() + jjj], data[iii*_map.size() + jjj], _frequency);
								APPL_VERBOSE("FB:  2 position -1: " << iii-1 << " " << data[(iii-1)*_map.size() + jjj]);
								APPL_VERBOSE("FB:  2 position  0: " << iii << " " << data[iii*_map.size() + jjj]);
								APPL_WARNING("FB:  2 time detected:     " << time << " delay = " << float((time-m_currentTick).count())/1000.0f << "µs");
							}
						} else if (m_stateFB == 3) {
							// TODO : Detect the pic ...
							// do nothing ...
						}
					}
				}
			}
			void onDataReceived(const void* _data,
			                    const std11::chrono::system_clock::time_point& _time,
			                    size_t _nbChunk,
			                    enum audio::format _format,
			                    uint32_t _frequency,
			                    const std::vector<audio::channel>& _map) {
				if (_format != audio::format_int16) {
					APPL_ERROR("call wrong type ... (need int16_t)");
				}
				RIVER_SAVE_FILE_MACRO(int16_t, "REC_Microphone.raw", _data, _nbChunk*_map.size());
				const int16_t* data = static_cast<const int16_t*>(_data);
				// Detect Zero crossing after a max/min ...
				for (size_t iii=0; iii<_nbChunk; ++iii) {
					//for (size_t jjj=0; jjj<_map.size(); ++jjj) {
					size_t jjj=0; {
						if (m_stateMic == 0) {
							if (data[iii*_map.size() + jjj] > 400/*INT16_MAX/15*/) {
								m_stateMic = 1;
								APPL_VERBOSE("Mic: detect Normal " << iii);
							} else if (data[iii*_map.size() + jjj] < -400/*INT16_MAX/15*/) {
								m_stateMic = 2;
								APPL_VERBOSE("Mic: detect inverse " << iii);
							}
						} else if (m_stateMic == 1) {
							// normale phase
							if (data[iii*_map.size() + jjj] <= 0) {
								// detect inversion of signe ...
								m_stateFB = 3;
								std11::chrono::system_clock::time_point time = getInterpolateTime(_time, iii-1, data[(iii-1)*_map.size() + jjj], data[iii*_map.size() + jjj], _frequency);
								APPL_VERBOSE("MIC: 1 position -1: " << iii-1 << " " << data[(iii-1)*_map.size() + jjj]);
								APPL_VERBOSE("MIC: 1 position  0: " << iii << " " << data[iii*_map.size() + jjj]);
								APPL_WARNING("MIC: 1 time detected:     " << time << " delay = " << float((time-m_currentTick).count())/1000.0f << "µs");
							}
						} else if (m_stateMic == 2) {
							// inverse phase
							if (data[iii*_map.size() + jjj] >= 0) {
								// detect inversion of signe ...
								m_stateMic = 3;
								std11::chrono::system_clock::time_point time = getInterpolateTime(_time, iii-1, data[(iii-1)*_map.size() + jjj], data[iii*_map.size() + jjj], _frequency);
								APPL_VERBOSE("MIC: 2 position -1: " << iii-1 << " " << data[(iii-1)*_map.size() + jjj]);
								APPL_VERBOSE("MIC: 2 position  0: " << iii << " " << data[iii*_map.size() + jjj]);
								APPL_WARNING("MIC: 2 time detected:     " << time << " delay = " << float((time-m_currentTick).count())/1000.0f << "µs");
							}
						} else if (m_stateMic == 3) {
							// TODO : Detect the pic ...
							// do nothing ...
						}
					}
				}
				/*
				const int16_t* data = static_cast<const int16_t*>(_data);
				int16_t value = 0;
				for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
					//APPL_INFO("value=" << data[iii]);
					value = std::max(int16_t(std::abs(data[iii])), value);
				}
				APPL_INFO("Get data ... max=" << value);
				*/
			}
			void run() {
				m_interfaceOut->start();
				m_interfaceIn->start();
				m_interfaceFB->start();
				usleep(30000000);
				m_interfaceFB->stop();
				m_interfaceIn->stop();
				m_interfaceOut->stop();
			}
	};
	
	TEST(TestTime, testDelay) {
		std11::shared_ptr<river::Manager> manager;
		manager = river::Manager::create("testApplication");
		std11::shared_ptr<TestClass> process = std11::make_shared<TestClass>(manager);
		process->run();
		process.reset();
		usleep(500000);
	}
};

#undef __class__
#define __class__ nullptr

#endif
