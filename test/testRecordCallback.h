/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_TEST_RECORD_CALLBACK_H__
#define __RIVER_TEST_RECORD_CALLBACK_H__

#include <audio/river/debug.h>

#undef __class__
#define __class__ "test_record_callback"

namespace river_test_record_callback {
	static const std::string configurationRiver =
		"{\n"
		"	microphone:{\n"
		"		io:'input',\n"
		"		map-on:{\n"
		"			interface:'auto',\n"
		"			name:'default',\n"
		"		},\n"
		"		frequency:0,\n"
		"		channel-map:['front-left', 'front-right'],\n"
		"		type:'auto',\n"
		"		nb-chunk:1024\n"
		"	}\n"
		"}\n";
	
	class testInCallback {
		public:
			std11::shared_ptr<audio::river::Manager> m_manager;
			std11::shared_ptr<audio::river::Interface> m_interface;
		public:
			testInCallback(std11::shared_ptr<audio::river::Manager> _manager, const std::string& _input="microphone") :
			  m_manager(_manager) {
				//Set stereo output:
				std::vector<audio::channel> channelMap;
				m_interface = m_manager->createInput(48000,
				                                     channelMap,
				                                     audio::format_int16,
				                                     _input);
				if(m_interface == nullptr) {
					APPL_ERROR("nullptr interface");
					return;
				}
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
				RIVER_SAVE_FILE_MACRO(int16_t, "REC_INPUT.raw", _data, _nbChunk * _map.size());
				const int16_t* data = static_cast<const int16_t*>(_data);
				int64_t value = 0;
				for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
					value += std::abs(data[iii]);
				}
				value /= (_nbChunk*_map.size());
				APPL_INFO("Get data ... average=" << int32_t(value));
			}
			void run() {
				if(m_interface == nullptr) {
					APPL_ERROR("nullptr interface");
					return;
				}
				m_interface->start();
				// wait 2 second ...
				usleep(20000000);
				m_interface->stop();
			}
	};
	
	TEST(TestALL, testInputCallBack) {
		audio::river::initString(configurationRiver);
		std11::shared_ptr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		APPL_INFO("test input (callback mode)");
		std11::shared_ptr<testInCallback> process = std11::make_shared<testInCallback>(manager);
		process->run();
		process.reset();
		usleep(500000);
		audio::river::unInit();
	}

};

#undef __class__
#define __class__ nullptr

#endif
