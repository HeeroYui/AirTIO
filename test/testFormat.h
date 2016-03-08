/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#undef __class__
#define __class__ "test_format"

namespace river_test_format {
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
	class testOutCallbackType {
		private:
			std::shared_ptr<audio::river::Manager> m_manager;
			std::shared_ptr<audio::river::Interface> m_interface;
			double m_phase;
			float m_freq;
			int32_t m_nbChannels;
			float m_generateFreq;
			
		public:
			testOutCallbackType(const std::shared_ptr<audio::river::Manager>& _manager,
			                    float _freq=48000.0f,
			                    int32_t _nbChannels=2,
			                    audio::format _format=audio::format_int16) :
			  m_manager(_manager),
			  m_phase(0),
			  m_freq(_freq),
			  m_nbChannels(_nbChannels),
			  m_generateFreq(550.0f) {
				//Set stereo output:
				std::vector<audio::channel> channelMap;
				if (m_nbChannels == 1) {
					channelMap.push_back(audio::channel_frontCenter);
				} else if (m_nbChannels == 2) {
					channelMap.push_back(audio::channel_frontLeft);
					channelMap.push_back(audio::channel_frontRight);
				} else if (m_nbChannels == 4) {
					channelMap.push_back(audio::channel_frontLeft);
					channelMap.push_back(audio::channel_frontRight);
					channelMap.push_back(audio::channel_rearLeft);
					channelMap.push_back(audio::channel_rearRight);
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
			                  const std::vector<audio::channel>& _map) {
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
				usleep(1000000);
				m_interface->stop();
				usleep(100000);
			}
	};
	
	
	class testResampling : public ::testing::TestWithParam<float> {};
	TEST_P(testResampling, base) {
		audio::river::initString(configurationRiver);
		std::shared_ptr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		std::shared_ptr<testOutCallbackType> process = std::make_shared<testOutCallbackType>(manager, GetParam(), 2, audio::format_int16);
		process->run();
		process.reset();
		usleep(500000);
		audio::river::unInit();
	}
	
	INSTANTIATE_TEST_CASE_P(InstantiationName,
	                        testResampling,
	                        ::testing::Values(4000, 8000, 16000, 32000, 48000, 48001, 64000, 96000, 11250, 2250, 44100, 88200));
	
	
	class testFormat : public ::testing::TestWithParam<audio::format> {};
	TEST_P(testFormat, base) {
		audio::river::initString(configurationRiver);
		std::shared_ptr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		std::shared_ptr<testOutCallbackType> process = std::make_shared<testOutCallbackType>(manager, 48000, 2, GetParam());
		process->run();
		process.reset();
		usleep(500000);
		audio::river::unInit();
	}
	INSTANTIATE_TEST_CASE_P(InstantiationName,
	                        testFormat,
	                        ::testing::Values(audio::format_int16, audio::format_int16_on_int32, audio::format_int32, audio::format_float));
	
	
	class testChannels : public ::testing::TestWithParam<int32_t> {};
	TEST_P(testChannels, base) {
		audio::river::initString(configurationRiver);
		std::shared_ptr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		std::shared_ptr<testOutCallbackType> process = std::make_shared<testOutCallbackType>(manager, 48000, GetParam(), audio::format_int16);
		process->run();
		process.reset();
		usleep(500000);
		audio::river::unInit();
	}
	INSTANTIATE_TEST_CASE_P(InstantiationName,
	                        testChannels,
	                        ::testing::Values(1,2,4));
	
	
	TEST(TestALL, testChannelsFormatResampling) {
		audio::river::initString(configurationRiver);
		std::shared_ptr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		TEST_INFO("test convert flaot to output (callback mode)");
		std::vector<float> listFreq;
		listFreq.push_back(4000);
		listFreq.push_back(8000);
		listFreq.push_back(16000);
		listFreq.push_back(32000);
		listFreq.push_back(48000);
		listFreq.push_back(48001);
		listFreq.push_back(64000);
		listFreq.push_back(96000);
		listFreq.push_back(11250);
		listFreq.push_back(2250);
		listFreq.push_back(44100);
		listFreq.push_back(88200);
		std::vector<int32_t> listChannel;
		listChannel.push_back(1);
		listChannel.push_back(2);
		listChannel.push_back(4);
		std::vector<audio::format> listFormat;
		listFormat.push_back(audio::format_int16);
		listFormat.push_back(audio::format_int16_on_int32);
		listFormat.push_back(audio::format_int32);
		listFormat.push_back(audio::format_float);
		for (size_t fff=0; fff<listFreq.size(); ++fff) {
			for (size_t ccc=0; ccc<listChannel.size(); ++ccc) {
				for (size_t iii=0; iii<listFormat.size(); ++iii) {
					TEST_INFO("freq=" << listFreq[fff] << " channel=" << listChannel[ccc] << " format=" << getFormatString(listFormat[iii]));
					std::shared_ptr<testOutCallbackType> process = std::make_shared<testOutCallbackType>(manager, listFreq[fff], listChannel[ccc], listFormat[iii]);
					process->run();
					process.reset();
					usleep(500000);
				}
			}
		}
		audio::river::unInit();
	}


};

#undef __class__
#define __class__ nullptr

