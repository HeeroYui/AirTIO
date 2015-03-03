/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_TEST_VOLUME_H__
#define __RIVER_TEST_VOLUME_H__

#undef __class__
#define __class__ "test_volume"

namespace river_test_volume {
	
	class testCallbackVolume {
		private:
			std11::shared_ptr<river::Manager> m_manager;
			std11::shared_ptr<river::Interface> m_interface;
			double m_phase;
		public:
			testCallbackVolume(std11::shared_ptr<river::Manager> _manager) :
			  m_manager(_manager),
			  m_phase(0) {
				//Set stereo output:
				std::vector<audio::channel> channelMap;
				channelMap.push_back(audio::channel_frontLeft);
				channelMap.push_back(audio::channel_frontRight);
				m_interface = m_manager->createOutput(48000,
				                                      channelMap,
				                                      audio::format_int16,
				                                      "speaker",
				                                      "WriteModeCallback");
				// set callback mode ...
				m_interface->setOutputCallback(std11::bind(&testCallbackVolume::onDataNeeded,
				                                           this,
				                                           std11::placeholders::_1,
				                                           std11::placeholders::_2,
				                                           std11::placeholders::_3,
				                                           std11::placeholders::_4,
				                                           std11::placeholders::_5,
				                                           std11::placeholders::_6));
				m_interface->addVolumeGroup("MEDIA");
				m_interface->addVolumeGroup("FLOW");
			}
			void onDataNeeded(void* _data,
			                  const std11::chrono::system_clock::time_point& _time,
			                  size_t _nbChunk,
			                  enum audio::format _format,
			                  uint32_t _frequency,
			                  const std::vector<audio::channel>& _map) {
				int16_t* data = static_cast<int16_t*>(_data);
				double baseCycle = 2.0*M_PI/(double)48000 * (double)550;
				for (int32_t iii=0; iii<_nbChunk; iii++) {
					for (int32_t jjj=0; jjj<_map.size(); jjj++) {
						data[_map.size()*iii+jjj] = cos(m_phase) * 30000;
					}
					m_phase += baseCycle;
					if (m_phase >= 2*M_PI) {
						m_phase -= 2*M_PI;
					}
				}
			}
			void run() {
				m_interface->start();
				usleep(1000000);
				m_interface->setParameter("volume", "FLOW", "-3dB");
				APPL_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				usleep(500000);
				m_interface->setParameter("volume", "FLOW", "-6dB");
				APPL_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				usleep(500000);
				m_interface->setParameter("volume", "FLOW", "-9dB");
				APPL_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				usleep(500000);
				m_interface->setParameter("volume", "FLOW", "-12dB");
				APPL_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				usleep(500000);
				m_interface->setParameter("volume", "FLOW", "-3dB");
				APPL_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				usleep(500000);
				m_interface->setParameter("volume", "FLOW", "3dB");
				APPL_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				usleep(500000);
				m_interface->setParameter("volume", "FLOW", "6dB");
				APPL_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				usleep(500000);
				m_interface->setParameter("volume", "FLOW", "9dB");
				APPL_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				usleep(500000);
				m_interface->setParameter("volume", "FLOW", "0dB");
				APPL_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				usleep(500000);
				m_manager->setVolume("MASTER", -3.0f);
				APPL_INFO("get volume MASTER: " << m_manager->getVolume("MASTER") );
				usleep(500000);
				m_manager->setVolume("MEDIA", -3.0f);
				APPL_INFO("get volume MEDIA: " << m_manager->getVolume("MEDIA") );
				usleep(1000000);
				m_interface->stop();
			}
	};
	
	TEST(TestALL, testVolume) {
		std11::shared_ptr<river::Manager> manager;
		manager = river::Manager::create("testApplication");
		std11::shared_ptr<testCallbackVolume> process = std11::make_shared<testCallbackVolume>(manager);
		process->run();
		process.reset();
		usleep(500000);
	}

};

#undef __class__
#define __class__ nullptr

#endif
