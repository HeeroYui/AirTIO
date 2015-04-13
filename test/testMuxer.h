/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#ifndef __RIVER_TEST_MUXER_H__
#define __RIVER_TEST_MUXER_H__

#include <audio/river/debug.h>

#undef __class__
#define __class__ "test_muxer"

namespace river_test_muxer {
	class TestClass {
		private:
			std11::shared_ptr<audio::river::Manager> m_manager;
			std11::shared_ptr<audio::river::Interface> m_interfaceIn;
			std11::shared_ptr<audio::river::Interface> m_interfaceOut;
			double m_phase;
		public:
			TestClass(std11::shared_ptr<audio::river::Manager> _manager) :
			  m_manager(_manager),
			  m_phase(0) {
				std::vector<audio::channel> channelMap;
				channelMap.push_back(audio::channel_frontLeft);
				channelMap.push_back(audio::channel_frontRight);
				m_interfaceOut = m_manager->createOutput(48000,
				                                         channelMap,
				                                         audio::format_int16,
				                                         "speaker");
				if(m_interfaceOut == nullptr) {
					APPL_ERROR("nullptr interface");
					return;
				}
				// set callback mode ...
				m_interfaceOut->setOutputCallback(std11::bind(&TestClass::onDataNeeded,
				                                              this,
				                                              std11::placeholders::_1,
				                                              std11::placeholders::_2,
				                                              std11::placeholders::_3,
				                                              std11::placeholders::_4,
				                                              std11::placeholders::_5,
				                                              std11::placeholders::_6));
				m_interfaceOut->addVolumeGroup("FLOW");
				//m_interfaceOut->setParameter("volume", "FLOW", "-6dB");
				
				//Set stereo output:
				m_interfaceIn = m_manager->createInput(48000,
				                                       std::vector<audio::channel>(),
				                                       audio::format_int16,
				                                       "microphone-muxed");
				if(m_interfaceIn == nullptr) {
					APPL_ERROR("nullptr interface");
					return;
				}
				// set callback mode ...
				m_interfaceIn->setInputCallback(std11::bind(&TestClass::onDataReceived,
				                                            this,
				                                            std11::placeholders::_1,
				                                            std11::placeholders::_2,
				                                            std11::placeholders::_3,
				                                            std11::placeholders::_4,
				                                            std11::placeholders::_5,
				                                            std11::placeholders::_6));
				m_manager->generateDotAll("activeProcess.dot");
			}
			
			void onDataNeeded(void* _data,
			                  const audio::Time& _time,
			                  size_t _nbChunk,
			                  enum audio::format _format,
			                  uint32_t _frequency,
			                  const std::vector<audio::channel>& _map) {
				int16_t* data = static_cast<int16_t*>(_data);
				double baseCycle = 2.0*M_PI/(double)48000 * 440;
				for (int32_t iii=0; iii<_nbChunk; iii++) {
					for (int32_t jjj=0; jjj<_map.size(); jjj++) {
						data[_map.size()*iii+jjj] = sin(m_phase) * 7000;
					}
					m_phase += baseCycle;
					if (m_phase >= 2*M_PI) {
						m_phase -= 2*M_PI;
					}
				}
			}
			void onDataReceived(const void* _data,
			                    const audio::Time& _time,
			                    size_t _nbChunk,
			                    enum audio::format _format,
			                    uint32_t _frequency,
			                    const std::vector<audio::channel>& _map) {
				if (_format != audio::format_int16) {
					APPL_ERROR("call wrong type ... (need int16_t)");
				}
				RIVER_SAVE_FILE_MACRO(int16_t, "REC_MicrophoneMuxed.raw", _data, _nbChunk*_map.size());
				APPL_ERROR("Receive data ... " << _nbChunk << " map=" << _map);
			}
			void run() {
				if(m_interfaceIn == nullptr) {
					APPL_ERROR("nullptr interface");
					return;
				}
				if(m_interfaceOut == nullptr) {
					APPL_ERROR("nullptr interface");
					return;
				}
				m_interfaceOut->start();
				m_interfaceIn->start();
				usleep(10000000);
				m_interfaceIn->stop();
				m_interfaceOut->stop();
			}
	};
	
	static const std::string configurationRiver = 
		"{\n"
		"	speaker:{\n"
		"		io:'output',\n"
		"		map-on:{\n"
		"			interface:'auto',\n"
		"			name:'default',\n"
		"			timestamp-mode:'trigered',\n"
		"		},\n"
		"		group:'groupSynchro',\n"
		"		frequency:0,\n"
		"		channel-map:['front-left', 'front-right'],\n"
		"		type:'auto',\n"
		"		nb-chunk:1024,\n"
		"	},\n"
		"	microphone:{\n"
		"		io:'input',\n"
		"		map-on:{\n"
		"			interface:'auto',\n"
		"			name:'default',\n"
		"			timestamp-mode:'trigered',\n"
		"		},\n"
		"		group:'groupSynchro',\n"
		"		frequency:0,\n"
		"		channel-map:['front-left', 'front-right'],\n"
		"		type:'auto',\n"
		"		nb-chunk:1024\n"
		"	},\n"
		"	microphone-muxed:{\n"
		"		io:'muxer',\n"
		"		# connect in input mode\n"
		"		map-on-input-1:{\n"
		"			# generic virtual definition\n"
		"			io:'input',\n"
		"			map-on:'microphone',\n"
		"		},\n"
		"		# connect in feedback mode\n"
		"		map-on-input-2:{\n"
		"			io:'feedback',\n"
		"			map-on:'speaker',\n"
		"		},\n"
		"		input-2-remap:['rear-left', 'rear-right'],\n"
		"		#classical format configuration:\n"
		"		frequency:48000,\n"
		"		channel-map:[\n"
		"			'front-left', 'front-right', 'rear-left', 'rear-right'\n"
		"		],\n"
		"		type:'int16',\n"
		"		mux-demux-type:'int16',\n"
		"	}\n"
		"}\n";
	
	TEST(TestMuxer, testMuxing) {
		audio::river::initString(configurationRiver);
		std11::shared_ptr<audio::river::Manager> manager;
		manager = audio::river::Manager::create("testApplication");
		std11::shared_ptr<TestClass> process = std11::make_shared<TestClass>(manager);
		process->run();
		process.reset();
		usleep(500000);
		audio::river::unInit();
	}
};

#undef __class__
#define __class__ nullptr

#endif
