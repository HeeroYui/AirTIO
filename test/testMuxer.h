/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_TEST_MUXER_H__
#define __RIVER_TEST_MUXER_H__

#include <river/debug.h>

#undef __class__
#define __class__ "test_muxer"

namespace river_test_muxer {
	class TestClass {
		private:
			std11::shared_ptr<river::Manager> m_manager;
			std11::shared_ptr<river::Interface> m_interfaceIn;
			std11::shared_ptr<river::Interface> m_interfaceOut;
			double m_phase;
		public:
			TestClass(std11::shared_ptr<river::Manager> _manager) :
			  m_manager(_manager),
			  m_phase(0) {
				std::vector<audio::channel> channelMap;
				channelMap.push_back(audio::channel_frontLeft);
				channelMap.push_back(audio::channel_frontRight);
				m_interfaceOut = m_manager->createOutput(48000,
				                                         channelMap,
				                                         audio::format_int16,
				                                         "speaker",
				                                         "MuxerTestOut");
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
				m_interfaceOut->setParameter("volume", "FLOW", "-6dB");
				
				//Set stereo output:
				m_interfaceIn = m_manager->createInput(48000,
				                                       std::vector<audio::channel>(),
				                                       audio::format_int16,
				                                       "microphone-muxed",
				                                       "microphone-muxed-local-name");
				// set callback mode ...
				m_interfaceIn->setInputCallback(std11::bind(&TestClass::onDataReceived,
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
				double baseCycle = 2.0*M_PI/(double)48000 * 440;
				for (int32_t iii=0; iii<_nbChunk; iii++) {
					for (int32_t jjj=0; jjj<_map.size(); jjj++) {
						data[_map.size()*iii+jjj] = sin(m_phase) * 30000;
					}
					m_phase += baseCycle;
					if (m_phase >= 2*M_PI) {
						m_phase -= 2*M_PI;
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
				RIVER_SAVE_FILE_MACRO(int16_t, "REC_MicrophoneMuxed.raw", _data, _nbChunk*_map.size());
				APPL_ERROR("Receive data ... " << _nbChunk << " map=" << _map);
			}
			void run() {
				m_interfaceOut->start();
				m_interfaceIn->start();
				usleep(10000000);
				m_interfaceIn->stop();
				m_interfaceOut->stop();
			}
	};
	
	TEST(TestMuxer, testMuxing) {
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
