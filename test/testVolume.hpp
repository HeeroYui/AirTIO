/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

namespace river_test_volume {
	static const std::string configurationRiver =
		"{\n"
		"	speaker:{\n"
		"		io:'output',\n"
		"		map-on:{\n"
		"			interface:'auto',\n"
		"			name:'default',\n"
		"		},\n"
		"		frequency:0,\n"
		"		channel-map:['front-left', 'front-right'],\n"
		"		type:'auto',\n"
		"		nb-chunk:1024,\n"
		"		volume-name:'MASTER'\n"
		"	}\n"
		"}\n";
	
	class testCallbackVolume {
		private:
			ememory::SharedPtr<audio::river::Manager> m_manager;
			ememory::SharedPtr<audio::river::Interface> m_interface;
			double m_phase;
		public:
			testCallbackVolume(ememory::SharedPtr<audio::river::Manager> _manager) :
			  m_manager(_manager),
			  m_phase(0) {
				//Set stereo output:
				std::vector<audio::channel> channelMap;
				channelMap.push_back(audio::channel_frontLeft);
				channelMap.push_back(audio::channel_frontRight);
				m_interface = m_manager->createOutput(48000,
				                                      channelMap,
				                                      audio::format_int16,
				                                      "speaker");
				if(m_interface == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				// set callback mode ...
				m_interface->setOutputCallback(std::bind(&testCallbackVolume::onDataNeeded,
				                                           this,
				                                           std::placeholders::_1,
				                                           std::placeholders::_2,
				                                           std::placeholders::_3,
				                                           std::placeholders::_4,
				                                           std::placeholders::_5,
				                                           std::placeholders::_6));
				m_interface->addVolumeGroup("MEDIA");
				m_interface->addVolumeGroup("FLOW");
			}
			void onDataNeeded(void* _data,
			                  const audio::Time& _time,
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
				if(m_interface == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				m_interface->start();
				std::this_thread::sleep_for(std::chrono::seconds(1));
				m_interface->setParameter("volume", "FLOW", "-3dB");
				TEST_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				m_interface->setParameter("volume", "FLOW", "-6dB");
				TEST_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				m_interface->setParameter("volume", "FLOW", "-9dB");
				TEST_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				m_interface->setParameter("volume", "FLOW", "-12dB");
				TEST_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				m_interface->setParameter("volume", "FLOW", "-3dB");
				TEST_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				m_interface->setParameter("volume", "FLOW", "3dB");
				TEST_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				m_interface->setParameter("volume", "FLOW", "6dB");
				TEST_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				m_interface->setParameter("volume", "FLOW", "9dB");
				TEST_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				m_interface->setParameter("volume", "FLOW", "0dB");
				TEST_INFO(" get volume : " << m_interface->getParameter("volume", "FLOW") );
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				m_manager->setVolume("MASTER", -3.0f);
				TEST_INFO("get volume MASTER: " << m_manager->getVolume("MASTER") );
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				m_manager->setVolume("MEDIA", -3.0f);
				TEST_INFO("get volume MEDIA: " << m_manager->getVolume("MEDIA") );
				std::this_thread::sleep_for(std::chrono::seconds(1));
				m_interface->stop();
			}
	};
	
	TEST(TestALL, testVolume) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		ememory::SharedPtr<testCallbackVolume> process = ememory::makeShared<testCallbackVolume>(manager);
		process->run();
		process.reset();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		audio::river::unInit();
	}

};


