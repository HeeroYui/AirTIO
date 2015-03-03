/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_TEST_PLAYBACK_WRITE_H__
#define __RIVER_TEST_PLAYBACK_WRITE_H__

#undef __class__
#define __class__ "test_playback_write"

namespace river_test_playback_write {
	class testOutWrite {
		private:
			std::vector<audio::channel> m_channelMap;
			std11::shared_ptr<river::Manager> m_manager;
			std11::shared_ptr<river::Interface> m_interface;
		public:
			testOutWrite(std11::shared_ptr<river::Manager> _manager) :
			  m_manager(_manager) {
				//Set stereo output:
				m_channelMap.push_back(audio::channel_frontLeft);
				m_channelMap.push_back(audio::channel_frontRight);
				m_interface = m_manager->createOutput(48000,
				                                      m_channelMap,
				                                      audio::format_int16,
				                                      "speaker",
				                                      "WriteMode");
				m_interface->setReadwrite();
			}
			void run() {
				double phase=0;
				std::vector<int16_t> data;
				data.resize(1024*m_channelMap.size());
				double baseCycle = 2.0*M_PI/48000.0 * 440.0;
				// start fill buffer
				for (int32_t kkk=0; kkk<10; ++kkk) {
					for (int32_t iii=0; iii<data.size()/m_channelMap.size(); iii++) {
						for (int32_t jjj=0; jjj<m_channelMap.size(); jjj++) {
							data[m_channelMap.size()*iii+jjj] = cos(phase) * 30000.0;
						}
						phase += baseCycle;
						if (phase >= 2*M_PI) {
							phase -= 2*M_PI;
						}
					}
					m_interface->write(&data[0], data.size()/m_channelMap.size());
				}
				m_interface->start();
				for (int32_t kkk=0; kkk<100; ++kkk) {
					for (int32_t iii=0; iii<data.size()/m_channelMap.size(); iii++) {
						for (int32_t jjj=0; jjj<m_channelMap.size(); jjj++) {
							data[m_channelMap.size()*iii+jjj] = cos(phase) * 30000.0;
						}
						phase += baseCycle;
						if (phase >= 2*M_PI) {
							phase -= 2*M_PI;
						}
					}
					m_interface->write(&data[0], data.size()/m_channelMap.size());
					// TODO : Add a function to get number of time we need to wait enought time ...
					usleep(15000);
				}
				m_interface->stop();
			}
	};
	
	TEST(TestALL, testOutputWrite) {
		std11::shared_ptr<river::Manager> manager;
		manager = river::Manager::create("testApplication");
		
		APPL_INFO("test output (write mode)");
		std11::shared_ptr<testOutWrite> process = std11::make_shared<testOutWrite>(manager);
		process->run();
		process.reset();
		usleep(500000);
	}
	
	class testOutWriteCallback {
		private:
			std11::shared_ptr<river::Manager> m_manager;
			std11::shared_ptr<river::Interface> m_interface;
			double m_phase;
		public:
			testOutWriteCallback(std11::shared_ptr<river::Manager> _manager) :
			  m_manager(_manager),
			  m_phase(0) {
				std::vector<audio::channel> channelMap;
				//Set stereo output:
				channelMap.push_back(audio::channel_frontLeft);
				channelMap.push_back(audio::channel_frontRight);
				m_interface = m_manager->createOutput(48000,
				                                      channelMap,
				                                      audio::format_int16,
				                                      "speaker",
				                                      "WriteMode+Callback");
				m_interface->setReadwrite();
				m_interface->setWriteCallback(std11::bind(&testOutWriteCallback::onDataNeeded,
				                                          this,
				                                          std11::placeholders::_1,
				                                          std11::placeholders::_2,
				                                          std11::placeholders::_3,
				                                          std11::placeholders::_4,
				                                          std11::placeholders::_5));
			}
			void onDataNeeded(const std11::chrono::system_clock::time_point& _time,
			                  size_t _nbChunk,
			                  enum audio::format _format,
			                  uint32_t _frequency,
			                  const std::vector<audio::channel>& _map) {
				if (_format != audio::format_int16) {
					APPL_ERROR("call wrong type ... (need int16_t)");
				}
				std::vector<int16_t> data;
				data.resize(1024*_map.size());
				double baseCycle = 2.0*M_PI/48000.0 * 440.0;
				// start fill buffer
				for (int32_t iii=0; iii<data.size()/_map.size(); iii++) {
					for (int32_t jjj=0; jjj<_map.size(); jjj++) {
						data[_map.size()*iii+jjj] = cos(m_phase) * 30000.0;
					}
					m_phase += baseCycle;
					if (m_phase >= 2*M_PI) {
						m_phase -= 2*M_PI;
					}
				}
				m_interface->write(&data[0], data.size()/_map.size());
			}
			void run() {
				m_interface->start();
				usleep(1000000);
				m_interface->stop();
			}
	};
	
	TEST(TestALL, testOutputWriteWithCallback) {
		std11::shared_ptr<river::Manager> manager;
		manager = river::Manager::create("testApplication");
		
		APPL_INFO("test output (write with callback event mode)");
		std11::shared_ptr<testOutWriteCallback> process = std11::make_shared<testOutWriteCallback>(manager);
		process->run();
		process.reset();
		usleep(500000);
	}

};

#undef __class__
#define __class__ nullptr

#endif
