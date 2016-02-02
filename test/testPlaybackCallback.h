/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#undef __class__
#define __class__ "test_playback_callback"

namespace river_test_playback_callback {
	
	class testOutCallback {
		public:
			std11::shared_ptr<audio::river::Manager> m_manager;
			std11::shared_ptr<audio::river::Interface> m_interface;
			double m_phase;
		public:
			testOutCallback(std11::shared_ptr<audio::river::Manager> _manager, const std::string& _io="speaker") :
			  m_manager(_manager),
			  m_phase(0) {
				//Set stereo output:
				std::vector<audio::channel> channelMap;
				channelMap.push_back(audio::channel_frontLeft);
				channelMap.push_back(audio::channel_frontRight);
				m_interface = m_manager->createOutput(48000,
				                                      channelMap,
				                                      audio::format_int16,
				                                      _io);
				if(m_interface == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				// set callback mode ...
				m_interface->setOutputCallback(std11::bind(&testOutCallback::onDataNeeded,
				                                           this,
				                                           std11::placeholders::_1,
				                                           std11::placeholders::_2,
				                                           std11::placeholders::_3,
				                                           std11::placeholders::_4,
				                                           std11::placeholders::_5,
				                                           std11::placeholders::_6));
			}
			void onDataNeeded(void* _data,
			                  const audio::Time& _time,
			                  size_t _nbChunk,
			                  enum audio::format _format,
			                  uint32_t _frequency,
			                  const std::vector<audio::channel>& _map) {
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
				usleep(2000000);
				m_interface->stop();
			}
	};
	
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
	
	TEST(TestALL, testOutputCallBack) {
		audio::river::initString(configurationRiver);
		std11::shared_ptr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		
		TEST_INFO("test output (callback mode)");
		std11::shared_ptr<testOutCallback> process = std11::make_shared<testOutCallback>(manager, "speaker");
		ASSERT_NE(process, nullptr);
		process->run();
		process.reset();
		usleep(500000);
		audio::river::unInit();
	}
	
	TEST(TestALL, testOutputCallBackPulse) {
		audio::river::initString(configurationRiver);
		std11::shared_ptr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		
		TEST_INFO("test output (callback mode)");
		std11::shared_ptr<testOutCallback> process = std11::make_shared<testOutCallback>(manager, "speaker-pulse");
		process->run();
		process.reset();
		usleep(500000);
		audio::river::unInit();
	}
	
	TEST(TestALL, testOutputCallBackJack) {
		audio::river::initString(configurationRiver);
		std11::shared_ptr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		
		TEST_INFO("test output (callback mode)");
		std11::shared_ptr<testOutCallback> process = std11::make_shared<testOutCallback>(manager, "speaker-jack");
		process->run();
		process.reset();
		usleep(500000);
		audio::river::unInit();
	}


};

#undef __class__
#define __class__ nullptr

