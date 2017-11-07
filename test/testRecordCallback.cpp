/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#include <test-debug/debug.hpp>
#include <audio/river/river.hpp>
#include <audio/river/Manager.hpp>
#include <audio/river/Interface.hpp>
#include <etest/etest.hpp>
#include <etk/etk.hpp>
#include <etk/os/FSNode.hpp>
extern "C" {
	#include <math.h>
}

#include <ethread/Thread.hpp>
#include <ethread/tools.hpp>
#include "main.hpp"

namespace river_test_record_callback {
	static const etk::String configurationRiver =
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
			ememory::SharedPtr<audio::river::Manager> m_manager;
			ememory::SharedPtr<audio::river::Interface> m_interface;
		public:
			testInCallback(ememory::SharedPtr<audio::river::Manager> _manager, const etk::String& _input="microphone") :
			  m_manager(_manager) {
				//Set stereo output:
				etk::Vector<audio::channel> channelMap;
				m_interface = m_manager->createInput(48000,
				                                     channelMap,
				                                     audio::format_int16,
				                                     _input);
				if(m_interface == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				// set callback mode ...
				m_interface->setInputCallback([=](const void* _data,
				                                    const audio::Time& _time,
				                                    size_t _nbChunk,
				                                    enum audio::format _format,
				                                    uint32_t _frequency,
				                                    const etk::Vector<audio::channel>& _map) {
				                                    	onDataReceived(_data, _time, _nbChunk, _format, _frequency, _map);
				                                    });
			}
			void onDataReceived(const void* _data,
			                    const audio::Time& _time,
			                    size_t _nbChunk,
			                    enum audio::format _format,
			                    uint32_t _frequency,
			                    const etk::Vector<audio::channel>& _map) {
				if (_format != audio::format_int16) {
					TEST_ERROR("call wrong type ... (need int16_t)");
				}
				TEST_SAVE_FILE_MACRO(int16_t, "REC_INPUT.raw", _data, _nbChunk * _map.size());
				const int16_t* data = static_cast<const int16_t*>(_data);
				int64_t value = 0;
				for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
					value += etk::abs(data[iii]);
				}
				value /= (_nbChunk*_map.size());
				TEST_INFO("Get data ... average=" << int32_t(value));
			}
			void run() {
				if(m_interface == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				m_interface->start();
				// wait 2 second ...
				ethread::sleepMilliSeconds(1000*(20));
				m_interface->stop();
			}
	};
	
	TEST(TestALL, testInputCallBack) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		TEST_INFO("test input (callback mode)");
		ememory::SharedPtr<testInCallback> process = ememory::makeShared<testInCallback>(manager);
		process->run();
		process.reset();
		ethread::sleepMilliSeconds((500));
		audio::river::unInit();
	}

};

