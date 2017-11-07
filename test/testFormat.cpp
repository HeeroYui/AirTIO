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

namespace river_test_format {
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
	class testOutCallbackType {
		private:
			ememory::SharedPtr<audio::river::Manager> m_manager;
			ememory::SharedPtr<audio::river::Interface> m_interface;
			double m_phase;
			float m_freq;
			int32_t m_nbChannels;
			float m_generateFreq;
			
		public:
			testOutCallbackType(const ememory::SharedPtr<audio::river::Manager>& _manager,
			                    float _freq=48000.0f,
			                    int32_t _nbChannels=2,
			                    audio::format _format=audio::format_int16) :
			  m_manager(_manager),
			  m_phase(0),
			  m_freq(_freq),
			  m_nbChannels(_nbChannels),
			  m_generateFreq(550.0f) {
				//Set stereo output:
				etk::Vector<audio::channel> channelMap;
				if (m_nbChannels == 1) {
					channelMap.pushBack(audio::channel_frontCenter);
				} else if (m_nbChannels == 2) {
					channelMap.pushBack(audio::channel_frontLeft);
					channelMap.pushBack(audio::channel_frontRight);
				} else if (m_nbChannels == 4) {
					channelMap.pushBack(audio::channel_frontLeft);
					channelMap.pushBack(audio::channel_frontRight);
					channelMap.pushBack(audio::channel_rearLeft);
					channelMap.pushBack(audio::channel_rearRight);
				} else {
					TEST_ERROR("Can not generate with channel != 1,2,4");
					return;
				}
				m_interface = m_manager->createOutput(m_freq,
				                                      channelMap,
				                                      _format,
				                                      "speaker",
				                                      "WriteModeCallbackType");
				if(m_interface == nullptr) {
					TEST_ERROR("nullptr interface");
					return;
				}
				// set callback mode ...
				m_interface->setOutputCallback([=](void* _data,
				                                   const audio::Time& _time,
				                                   size_t _nbChunk,
				                                   enum audio::format _format,
				                                   uint32_t _frequency,
				                                   const etk::Vector<audio::channel>& _map) {
				                                   	onDataNeeded(_data, _time, _nbChunk, _format, _frequency, _map);
				                                   });
			}
			void onDataNeeded(void* _data,
			                  const audio::Time& _time,
			                  size_t _nbChunk,
			                  enum audio::format _format,
			                  uint32_t _frequency,
			                  const etk::Vector<audio::channel>& _map) {
				//TEST_DEBUG("Get data ... " << _format << " map=" << _map << " chunk=" << _nbChunk);
				double baseCycle = 2.0*M_PI/double(m_freq) * double(m_generateFreq);
				if (_format == audio::format_int16) {
					int16_t* data = static_cast<int16_t*>(_data);
					for (int32_t iii=0; iii<_nbChunk; iii++) {
						for (int32_t jjj=0; jjj<_map.size(); jjj++) {
							data[_map.size()*iii+jjj] = cos(m_phase) * double(INT16_MAX);
						}
						m_phase += baseCycle;
						if (m_phase >= 2*M_PI) {
							m_phase -= 2*M_PI;
						}
					}
				} else if (_format == audio::format_int16_on_int32) {
					int32_t* data = static_cast<int32_t*>(_data);
					for (int32_t iii=0; iii<_nbChunk; iii++) {
						for (int32_t jjj=0; jjj<_map.size(); jjj++) {
							data[_map.size()*iii+jjj] = cos(m_phase) * double(INT16_MAX);
						}
						m_phase += baseCycle;
						if (m_phase >= 2*M_PI) {
							m_phase -= 2*M_PI;
						}
					}
				} else if (_format == audio::format_int32) {
					int32_t* data = static_cast<int32_t*>(_data);
					for (int32_t iii=0; iii<_nbChunk; iii++) {
						for (int32_t jjj=0; jjj<_map.size(); jjj++) {
							data[_map.size()*iii+jjj] = cos(m_phase) * double(INT32_MAX);
						}
						m_phase += baseCycle;
						if (m_phase >= 2*M_PI) {
							m_phase -= 2*M_PI;
						}
					}
				} else if (_format == audio::format_float) {
					float* data = static_cast<float*>(_data);
					for (int32_t iii=0; iii<_nbChunk; iii++) {
						for (int32_t jjj=0; jjj<_map.size(); jjj++) {
							data[_map.size()*iii+jjj] = cos(m_phase);
						}
						m_phase += baseCycle;
						if (m_phase >= 2*M_PI) {
							m_phase -= 2*M_PI;
						}
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
				ethread::sleepMilliSeconds(1000*(1));
				m_interface->stop();
				ethread::sleepMilliSeconds((100));
			}
	};
	etk::Vector<float> listFreq = {4000, 8000, 16000, 32000, 48000, 48001, 64000, 96000, 11250, 2250, 44100, 88200};
	etk::Vector<int32_t> listChannel = {1, 2, 4};
	etk::Vector<audio::format> listFormat = {audio::format_int16, audio::format_int16_on_int32, audio::format_int32, audio::format_float};
	
	TEST(testResampling, base) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		for (auto itFrequency: listFreq) {
			ememory::SharedPtr<testOutCallbackType> process = ememory::makeShared<testOutCallbackType>(manager, itFrequency, 2, audio::format_int16);
			process->run();
			process.reset();
			ethread::sleepMilliSeconds((500));
		}
		audio::river::unInit();
	}
	
	TEST(testFormat, base) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		for (auto itFormat: listFormat) {
			ememory::SharedPtr<testOutCallbackType> process = ememory::makeShared<testOutCallbackType>(manager, 48000, 2, itFormat);
			process->run();
			process.reset();
			ethread::sleepMilliSeconds((500));
		}
		audio::river::unInit();
	}
	
	TEST(testChannels, base) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		for (auto itChannel: listChannel) {
			ememory::SharedPtr<testOutCallbackType> process = ememory::makeShared<testOutCallbackType>(manager, 48000, itChannel, audio::format_int16);
			process->run();
			process.reset();
			ethread::sleepMilliSeconds((500));
		}
		audio::river::unInit();
	}
	
	TEST(TestALL, testChannelsFormatResampling) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		TEST_INFO("test convert flaot to output (callback mode)");
		for (auto itFrequency: listFreq) {
			for (auto itChannel: listChannel) {
				for (auto itFormat: listFormat) {
					TEST_INFO("freq=" << itFrequency << " channel=" << listChannel << " format=" << audio::getFormatString(itFormat));
					ememory::SharedPtr<testOutCallbackType> process = ememory::makeShared<testOutCallbackType>(manager, itFrequency, itChannel, itFormat);
					process->run();
					process.reset();
					ethread::sleepMilliSeconds((500));
				}
			}
		}
		audio::river::unInit();
	}
}


