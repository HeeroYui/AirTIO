/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_TEST_RECORD_CALLBACK_H__
#define __RIVER_TEST_RECORD_CALLBACK_H__

#undef __class__
#define __class__ "test_record_callback"

namespace river_test_record_callback {
	class testInCallback {
		private:
			std11::shared_ptr<river::Manager> m_manager;
			std11::shared_ptr<river::Interface> m_interface;
			double m_phase;
		public:
			testInCallback(std11::shared_ptr<river::Manager> _manager, const std::string& _input="microphone") :
			  m_manager(_manager),
			  m_phase(0) {
				//Set stereo output:
				std::vector<audio::channel> channelMap;
				channelMap.push_back(audio::channel_frontLeft);
				channelMap.push_back(audio::channel_frontRight);
				m_interface = m_manager->createInput(48000,
				                                     channelMap,
				                                     audio::format_int16,
				                                     _input,
				                                     "WriteModeCallback");
				// set callback mode ...
				m_interface->setInputCallback(std11::bind(&testInCallback::onDataReceived,
				                                          this,
				                                          std11::placeholders::_1,
				                                          std11::placeholders::_2,
				                                          std11::placeholders::_3,
				                                          std11::placeholders::_4,
				                                          std11::placeholders::_5,
				                                          std11::placeholders::_6));
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
				const int16_t* data = static_cast<const int16_t*>(_data);
				int64_t value = 0;
				for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
					value += std::abs(data[iii]);
				}
				value /= (_nbChunk*_map.size());
				APPL_INFO("Get data ... average=" << int32_t(value));
			}
			void run() {
				m_interface->start();
				// wait 2 second ...
				usleep(2000000);
				
				m_manager->generateDotAll("activeProcess.dot");
				m_interface->stop();
			}
	};
	
	TEST(TestALL, testInputCallBack) {
		std11::shared_ptr<river::Manager> manager;
		manager = river::Manager::create("testApplication");
		APPL_INFO("test input (callback mode)");
		std11::shared_ptr<testInCallback> process = std11::make_shared<testInCallback>(manager);
		process->run();
		process.reset();
		usleep(500000);
	}

};

#undef __class__
#define __class__ nullptr

#endif
