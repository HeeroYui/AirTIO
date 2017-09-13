/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

namespace river_test_playback_callback {
	
	class testOutCallback {
		public:
			ememory::SharedPtr<audio::river::Manager> m_manager;
			ememory::SharedPtr<audio::river::Interface> m_interface;
			double m_phase;
		public:
			testOutCallback(ememory::SharedPtr<audio::river::Manager> _manager, const etk::String& _io="speaker") :
			  m_manager(_manager),
			  m_phase(0) {
				//Set stereo output:
				etk::Vector<audio::channel> channelMap;
				channelMap.pushBack(audio::channel_frontLeft);
				channelMap.pushBack(audio::channel_frontRight);
				m_interface = m_manager->createOutput(48000,
				                                      channelMap,
				                                      audio::format_int16,
				                                      _io);
				if(m_interface == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				// set callback mode ...
				m_interface->setOutputCallback(std::bind(&testOutCallback::onDataNeeded,
				                                           this,
				                                           std::placeholders::_1,
				                                           std::placeholders::_2,
				                                           std::placeholders::_3,
				                                           std::placeholders::_4,
				                                           std::placeholders::_5,
				                                           std::placeholders::_6));
			}
			void onDataNeeded(void* _data,
			                  const audio::Time& _time,
			                  size_t _nbChunk,
			                  enum audio::format _format,
			                  uint32_t _frequency,
			                  const etk::Vector<audio::channel>& _map) {
				if (_format != audio::format_int16) {
					TEST_ERROR("call wrong type ... (need int16_t)");
				}
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
				// wait 2 second ...
				ethread::sleepMilliSeconds(std::chrono::seconds(2));
				m_interface->stop();
			}
	};
	
	static const etk::String configurationRiver =
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
	
	TEST(TestALL, testOutputCallBack) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		
		TEST_INFO("test output (callback mode)");
		ememory::SharedPtr<testOutCallback> process = ememory::makeShared<testOutCallback>(manager, "speaker");
		ASSERT_NE(process, nullptr);
		process->run();
		process.reset();
		ethread::sleepMilliSeconds((500));
		audio::river::unInit();
	}
	
	TEST(TestALL, testOutputCallBackPulse) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		
		TEST_INFO("test output (callback mode)");
		ememory::SharedPtr<testOutCallback> process = ememory::makeShared<testOutCallback>(manager, "speaker-pulse");
		process->run();
		process.reset();
		ethread::sleepMilliSeconds((500));
		audio::river::unInit();
	}
	
	TEST(TestALL, testOutputCallBackJack) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		
		TEST_INFO("test output (callback mode)");
		ememory::SharedPtr<testOutCallback> process = ememory::makeShared<testOutCallback>(manager, "speaker-jack");
		process->run();
		process.reset();
		ethread::sleepMilliSeconds((500));
		audio::river::unInit();
	}
};

