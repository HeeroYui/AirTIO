/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

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
				m_interface->setOutputCallback(std::bind(&testOutCallbackType::onDataNeeded,
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
				std::this_thread::sleep_for(std::chrono::seconds(1));
				m_interface->stop();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
	};
	
	
	class testResampling : public ::testing::TestWithParam<float> {};
	TEST_P(testResampling, base) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		ememory::SharedPtr<testOutCallbackType> process = ememory::makeShared<testOutCallbackType>(manager, GetParam(), 2, audio::format_int16);
		process->run();
		process.reset();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		audio::river::unInit();
	}
	
	INSTANTIATE_TEST_CASE_P(InstantiationName,
	                        testResampling,
	                        ::testing::Values(4000, 8000, 16000, 32000, 48000, 48001, 64000, 96000, 11250, 2250, 44100, 88200));
	
	
	class testFormat : public ::testing::TestWithParam<audio::format> {};
	TEST_P(testFormat, base) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		ememory::SharedPtr<testOutCallbackType> process = ememory::makeShared<testOutCallbackType>(manager, 48000, 2, GetParam());
		process->run();
		process.reset();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		audio::river::unInit();
	}
	INSTANTIATE_TEST_CASE_P(InstantiationName,
	                        testFormat,
	                        ::testing::Values(audio::format_int16, audio::format_int16_on_int32, audio::format_int32, audio::format_float));
	
	
	class testChannels : public ::testing::TestWithParam<int32_t> {};
	TEST_P(testChannels, base) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		ememory::SharedPtr<testOutCallbackType> process = ememory::makeShared<testOutCallbackType>(manager, 48000, GetParam(), audio::format_int16);
		process->run();
		process.reset();
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		audio::river::unInit();
	}
	INSTANTIATE_TEST_CASE_P(InstantiationName,
	                        testChannels,
	                        ::testing::Values(1,2,4));
	
	
	TEST(TestALL, testChannelsFormatResampling) {
		audio::river::initString(configurationRiver);
		ememory::SharedPtr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		TEST_INFO("test convert flaot to output (callback mode)");
		etk::Vector<float> listFreq;
		listFreq.pushBack(4000);
		listFreq.pushBack(8000);
		listFreq.pushBack(16000);
		listFreq.pushBack(32000);
		listFreq.pushBack(48000);
		listFreq.pushBack(48001);
		listFreq.pushBack(64000);
		listFreq.pushBack(96000);
		listFreq.pushBack(11250);
		listFreq.pushBack(2250);
		listFreq.pushBack(44100);
		listFreq.pushBack(88200);
		etk::Vector<int32_t> listChannel;
		listChannel.pushBack(1);
		listChannel.pushBack(2);
		listChannel.pushBack(4);
		etk::Vector<audio::format> listFormat;
		listFormat.pushBack(audio::format_int16);
		listFormat.pushBack(audio::format_int16_on_int32);
		listFormat.pushBack(audio::format_int32);
		listFormat.pushBack(audio::format_float);
		for (size_t fff=0; fff<listFreq.size(); ++fff) {
			for (size_t ccc=0; ccc<listChannel.size(); ++ccc) {
				for (size_t iii=0; iii<listFormat.size(); ++iii) {
					TEST_INFO("freq=" << listFreq[fff] << " channel=" << listChannel[ccc] << " format=" << getFormatString(listFormat[iii]));
					ememory::SharedPtr<testOutCallbackType> process = ememory::makeShared<testOutCallbackType>(manager, listFreq[fff], listChannel[ccc], listFormat[iii]);
					process->run();
					process.reset();
					std::this_thread::sleep_for(std::chrono::milliseconds(500));
				}
			}
		}
		audio::river::unInit();
	}
};


