/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <test-debug/debug.hpp>

namespace river_test_echo_delay {
	class TestClass {
		private:
			ememory::SharedPtr<audio::river::Manager> m_manager;
			ememory::SharedPtr<audio::river::Interface> m_interfaceOut;
			ememory::SharedPtr<audio::river::Interface> m_interfaceIn;
			ememory::SharedPtr<audio::river::Interface> m_interfaceFB;
			double m_phase;
			double m_freq;
			int32_t m_nextSampleCount;
			std::chrono::milliseconds m_delayBetweenEvent;
			audio::Time m_nextTick;
			audio::Time m_currentTick;
			int32_t m_stateFB;
			int32_t m_stateMic;
			std::vector<uint64_t> m_delayListMic;
			bool m_estimateVolumeInput;
			int16_t m_volumeInputMax;
			int16_t m_volumeInputMin;
			float m_gain;
		public:
			TestClass(ememory::SharedPtr<audio::river::Manager> _manager) :
			  m_manager(_manager),
			  m_phase(0),
			  m_freq(400),
			  m_nextSampleCount(0),
			  m_delayBetweenEvent(400),
			  m_stateFB(3),
			  m_stateMic(0),
			  m_estimateVolumeInput(true),
			  m_gain(-40) {
				//Set stereo output:
				std::vector<audio::channel> channelMap;
				m_interfaceOut = m_manager->createOutput(48000,
				                                         channelMap,
				                                         audio::format_int16,
				                                         "speaker");
				if(m_interfaceOut == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				// set callback mode ...
				m_interfaceOut->setOutputCallback(std::bind(&TestClass::onDataNeeded,
				                                              this,
				                                              std::placeholders::_1,
				                                              std::placeholders::_2,
				                                              std::placeholders::_3,
				                                              std::placeholders::_4,
				                                              std::placeholders::_5,
				                                              std::placeholders::_6));
				m_interfaceOut->addVolumeGroup("FLOW");
				m_interfaceOut->setParameter("volume", "FLOW", etk::to_string(m_gain) + "dB");
				
				m_interfaceIn = m_manager->createInput(48000,
				                                       channelMap,
				                                       audio::format_int16,
				                                       "microphone");
				if(m_interfaceIn == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				// set callback mode ...
				m_interfaceIn->setInputCallback(std::bind(&TestClass::onDataReceived,
				                                            this,
				                                            std::placeholders::_1,
				                                            std::placeholders::_2,
				                                            std::placeholders::_3,
				                                            std::placeholders::_4,
				                                            std::placeholders::_5,
				                                            std::placeholders::_6));
				
				m_interfaceFB = m_manager->createFeedback(48000,
				                                          channelMap,
				                                          audio::format_int16,
				                                          "speaker");
				if(m_interfaceFB == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				// set callback mode ...
				m_interfaceFB->setInputCallback(std::bind(&TestClass::onDataReceivedFeedBack,
				                                            this,
				                                            std::placeholders::_1,
				                                            std::placeholders::_2,
				                                            std::placeholders::_3,
				                                            std::placeholders::_4,
				                                            std::placeholders::_5,
				                                            std::placeholders::_6));
				
				m_manager->generateDotAll("activeProcess.dot");
			}
			void onDataNeeded(void* _data,
			                  const audio::Time& _time,
			                  size_t _nbChunk,
			                  enum audio::format _format,
			                  uint32_t _frequency,
			                  const std::vector<audio::channel>& _map) {
				int16_t* data = static_cast<int16_t*>(_data);
				double baseCycle = 2.0*M_PI/(double)48000 * m_freq;
				if (m_estimateVolumeInput == true) {
					for (int32_t iii=0; iii<_nbChunk; iii++) {
						for (int32_t jjj=0; jjj<_map.size(); jjj++) {
							data[_map.size()*iii+jjj] = sin(m_phase) * 30000;
						}
						m_phase += baseCycle;
						if (m_phase >= 2*M_PI) {
							m_phase -= 2*M_PI;
						}
					}
				} else {
					if (_time == audio::Time()) {
						for (int32_t iii=0; iii<_nbChunk; iii++) {
							for (int32_t jjj=0; jjj<_map.size(); jjj++) {
								data[_map.size()*iii+jjj] = 0;
							}
						}
						return;
					}
					if (m_nextTick == audio::Time()) {
						m_nextTick = _time + m_delayBetweenEvent;
						m_nextSampleCount = m_delayBetweenEvent.count()*int64_t(_frequency)/1000;
						m_phase = -1;
					}
					//TEST_INFO("sample : " << m_nextSampleCount);
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
								TEST_WARNING("Time Pulse zero crossing: " << m_currentTick << " id=" << iii);
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
			}
			audio::Time getInterpolateTime(audio::Time _time, int32_t _pos, int16_t _val1, int16_t _val2, uint32_t _frequency) {
				if (_val1 == 0) {
					return _time + audio::Duration(0, int64_t(_pos)*1000000000LL/int64_t(_frequency));
				} else if (_val2 == 0) {
					return _time + audio::Duration(0, int64_t(_pos+1)*1000000000LL/int64_t(_frequency));
				}
				double xxx = double(-_val1) / double(_val2 - _val1);
				TEST_VERBOSE("deltaPos:" << xxx);
				return _time + audio::Duration(0, int64_t((double(_pos)+xxx)*1000000000.0)/int64_t(_frequency));
			}
			
			void onDataReceivedFeedBack(const void* _data,
			                            const audio::Time& _time,
			                            size_t _nbChunk,
			                            enum audio::format _format,
			                            uint32_t _frequency,
			                            const std::vector<audio::channel>& _map) {
				if (_format != audio::format_int16) {
					TEST_ERROR("call wrong type ... (need int16_t)");
				}
				TEST_SAVE_FILE_MACRO(int16_t, "REC_FeedBack.raw", _data, _nbChunk*_map.size());
				if (m_estimateVolumeInput == true) {
					// nothing to do ...
				} else {
					const int16_t* data = static_cast<const int16_t*>(_data);
					// Detect Zero crossing after a max/min ...
					for (size_t iii=0; iii<_nbChunk; ++iii) {
						//for (size_t jjj=0; jjj<_map.size(); ++jjj) {
						size_t jjj=0; {
							if (m_stateFB == 0) {
								if (data[iii*_map.size() + jjj] > INT16_MAX/5) {
									m_stateFB = 1;
									TEST_VERBOSE("FB: detect Normal " << iii);
								} else if (data[iii*_map.size() + jjj] < -INT16_MAX/5) {
									m_stateFB = 2;
									TEST_VERBOSE("FB: detect inverse " << iii);
								}
							} else if (m_stateFB == 1) {
								// normale phase
								if (data[iii*_map.size() + jjj] <= 0) {
									// detect inversion of signe ...
									m_stateFB = 3;
									audio::Time time = getInterpolateTime(_time, iii-1, data[(iii-1)*_map.size() + jjj], data[iii*_map.size() + jjj], _frequency);
									TEST_VERBOSE("FB:  1 position -1:  " << iii-1 << " " << data[(iii-1)*_map.size() + jjj]);
									TEST_VERBOSE("FB:  1 position  0: " << iii << " " << data[iii*_map.size() + jjj]);
									
									TEST_WARNING("FB:  1 time detected:     " << time << " delay = " << float((time-m_currentTick).count())/1000.0f << "µs");
								}
							} else if (m_stateFB == 2) {
								// inverse phase
								if (data[iii*_map.size() + jjj] >= 0) {
									// detect inversion of signe ...
									m_stateFB = 3;
									audio::Time time = getInterpolateTime(_time, iii-1, data[(iii-1)*_map.size() + jjj], data[iii*_map.size() + jjj], _frequency);
									TEST_VERBOSE("FB:  2 position -1: " << iii-1 << " " << data[(iii-1)*_map.size() + jjj]);
									TEST_VERBOSE("FB:  2 position  0: " << iii << " " << data[iii*_map.size() + jjj]);
									TEST_WARNING("FB:  2 time detected:     " << time << " delay = " << float((time-m_currentTick).count())/1000.0f << "µs");
								}
							} else if (m_stateFB == 3) {
								// TODO : Detect the pic ...
								// do nothing ...
							}
						}
					}
				}
			}
			void onDataReceived(const void* _data,
			                    const audio::Time& _time,
			                    size_t _nbChunk,
			                    enum audio::format _format,
			                    uint32_t _frequency,
			                    const std::vector<audio::channel>& _map) {
				if (_format != audio::format_int16) {
					TEST_ERROR("call wrong type ... (need int16_t)");
				}
				TEST_SAVE_FILE_MACRO(int16_t, "REC_Microphone.raw", _data, _nbChunk*_map.size());
				const int16_t* data = static_cast<const int16_t*>(_data);
				if (m_estimateVolumeInput == true) {
					m_stateMic ++;
					const int16_t* data = static_cast<const int16_t*>(_data);
					if (m_stateMic <= 40) {
						for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
							//TEST_INFO("value=" << data[iii]);
							m_volumeInputMax = std::max(int16_t(data[iii]), m_volumeInputMax);
							m_volumeInputMin = std::min(int16_t(data[iii]), m_volumeInputMin);
						}
						if (m_stateMic == 40) {
							m_volumeInputMax *= 2;
							m_volumeInputMin *= 2;
						}
					} else if (m_stateMic <= 10000) {
						int16_t valueMax = 0;
						int16_t valueMin = 0;
						for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
							//TEST_INFO("value=" << data[iii]);
							valueMax = std::max(int16_t(data[iii]), valueMax);
							valueMin = std::min(int16_t(data[iii]), valueMin);
						}
						if (    valueMax > m_volumeInputMax
						     && valueMin < m_volumeInputMin
						     && (    m_gain == 0.0
						          || (    valueMax > INT16_MAX*2/3
						               && valueMin < INT16_MIN*2/3
						             )
						         )
						   ) {
							m_gain += 3.0f;
							m_gain = std::min(m_gain, 0.0f);
							m_interfaceOut->setParameter("volume", "FLOW", etk::to_string(m_gain) + "dB");
							TEST_INFO("Set detection volume : " << m_gain << " m_stateMic=" << m_stateMic);
							m_stateMic = 3;
							m_phase = -1;
							m_estimateVolumeInput = false;
							return;
						} else {
							if (m_stateMic%2 == 0) {
								if (m_gain == 0.0f) {
									TEST_CRITICAL("Can not find the basicVolume ...");
								}
								// just update volume
								m_gain += 1.0f;
								m_gain = std::min(m_gain, 0.0f);
								m_interfaceOut->setParameter("volume", "FLOW", etk::to_string(m_gain) + "dB");
							}
						}
					}
				} else {
					// Detect Zero crossing after a max/min ...
					for (size_t iii=0; iii<_nbChunk; ++iii) {
						//for (size_t jjj=0; jjj<_map.size(); ++jjj) {
						size_t jjj=0; {
							if (m_stateMic == 0) {
								if (data[iii*_map.size() + jjj] > m_volumeInputMax) {
									m_stateMic = 1;
									TEST_VERBOSE("Mic: detect Normal " << iii);
								} else if (data[iii*_map.size() + jjj] < m_volumeInputMin) {
									m_stateMic = 2;
									TEST_VERBOSE("Mic: detect inverse " << iii);
								}
							} else if (m_stateMic == 1) {
								// normale phase
								if (data[iii*_map.size() + jjj] <= 0) {
									// detect inversion of signe ...
									m_stateMic = 3;
									audio::Time time = getInterpolateTime(_time, iii-1, data[(iii-1)*_map.size() + jjj], data[iii*_map.size() + jjj], _frequency);
									TEST_VERBOSE("MIC: 1 position -1: " << iii-1 << " " << data[(iii-1)*_map.size() + jjj]);
									TEST_VERBOSE("MIC: 1 position  0: " << iii << " " << data[iii*_map.size() + jjj]);
									audio::Duration delay = time-m_currentTick;
									int32_t sampleDalay = (delay.count()*_frequency)/1000000000LL;
									TEST_WARNING("MIC: 1 time detected:     " << time << " delay = " << float(delay.count())/1000.0f << "µs  samples=" << sampleDalay);
									m_delayListMic.push_back(delay.count());
								}
							} else if (m_stateMic == 2) {
								// inverse phase
								if (data[iii*_map.size() + jjj] >= 0) {
									// detect inversion of signe ...
									m_stateMic = 3;
									audio::Time time = getInterpolateTime(_time, iii-1, data[(iii-1)*_map.size() + jjj], data[iii*_map.size() + jjj], _frequency);
									TEST_VERBOSE("MIC: 2 position -1: " << iii-1 << " " << data[(iii-1)*_map.size() + jjj]);
									TEST_VERBOSE("MIC: 2 position  0: " << iii << " " << data[iii*_map.size() + jjj]);
									audio::Duration delay = time-m_currentTick;
									int32_t sampleDalay = (delay.count()*_frequency)/1000000000LL;
									TEST_WARNING("MIC: 2 time detected:     " << time << " delay = " << float(delay.count())/1000.0f << "µs  samples=" << sampleDalay);
									m_delayListMic.push_back(delay.count());
								}
							} else if (m_stateMic == 3) {
								// TODO : Detect the pic ...
								// do nothing ...
							}
						}
					}
				}
			}
			void run() {
				if(m_interfaceIn == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				if(m_interfaceOut == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				if(m_interfaceFB == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				m_interfaceOut->start();
				m_interfaceIn->start();
				//m_interfaceFB->start();
				while (m_estimateVolumeInput == true) {
					usleep(10000);
				}
				usleep(10000000);
				//m_interfaceFB->stop();
				m_interfaceIn->stop();
				m_interfaceOut->stop();
				
				int64_t delayAverage = 0;
				if (m_delayListMic.size() > 0) {
					for (size_t iii=0; iii<m_delayListMic.size(); ++iii) {
						delayAverage += m_delayListMic[iii];
					}
					delayAverage /= m_delayListMic.size();
				}
				int32_t sampleDalay = (delayAverage*48000)/1000000000LL;
				TEST_ERROR("Average delay in ns : " << delayAverage << " nbSample=" << sampleDalay);
			}
	};
	
	static const std::string configurationRiver =
		"{\n"
		"	speaker:{\n"
		"		io:'output',\n"
		"		map-on:{\n"
		"			interface:'auto',\n"
		"			name:'default',\n"
		//"			name:'hw:0,0',\n"
		"			timestamp-mode:'trigered',\n"
		"		},\n"
		//"		group:'groupSynchro',\n"
		"		frequency:0,\n"
		"		channel-map:['front-left', 'front-right'],\n"
		"		type:'auto',\n"
		"		nb-chunk:1024,\n"
		"	},\n"
		"	microphone:{\n"
		"		io:'input',\n"
		"		map-on:{\n"
		"			interface:'auto',\n"
		"			name:'default',\n"
		//"			name:'hw:0,0',\n"
		"			timestamp-mode:'trigered',\n"
		"		},\n"
		//"		group:'groupSynchro',\n"
		"		frequency:0,\n"
		"		channel-map:['front-left', 'front-right'],\n"
		"		type:'auto',\n"
		"		nb-chunk:1024\n"
		"	}\n"
		"}\n";
	
	TEST(TestTime, testDelay) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		ememory::SharedPtr<TestClass> process = ememory::makeShared<TestClass>(manager);
		process->run();
		process.reset();
		usleep(500000);
		audio::river::unInit();
	}
};


