/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include "debug.h"
#include <airtio/Manager.h>
#include <airtio/Interface.h>
#include <gtest/gtest.h>
#include <etk/os/FSNode.h>

#include <sstream>
#include <unistd.h>

#undef __class__
#define __class__ "test"

class testOutWrite {
	private:
		std::vector<airtalgo::channel> m_channelMap;
		std::shared_ptr<airtio::Manager> m_manager;
		std::shared_ptr<airtio::Interface> m_interface;
	public:
		testOutWrite(std::shared_ptr<airtio::Manager> _manager) :
		  m_manager(_manager) {
			//Set stereo output:
			m_channelMap.push_back(airtalgo::channel_frontLeft);
			m_channelMap.push_back(airtalgo::channel_frontRight);
			m_interface = m_manager->createOutput(48000,
			                                      m_channelMap,
			                                      airtalgo::format_int16,
			                                      "default",
			                                      "WriteMode");
		}
		void run() {
			double phase=0;
			std::vector<int16_t> data;
			data.resize(1024*m_channelMap.size());
			double baseCycle = 2.0*M_PI/48000.0 * 440.0;
			// start fill buffer
			for (int32_t kkk=0; kkk<10; ++kkk) {
				for (int32_t iii=0; iii<data.size()/m_channelMap.size(); iii++) {
					for (int32_t jjj=0; jjj<m_channelMap.size(); jjj++) {
						data[m_channelMap.size()*iii+jjj] = cos(phase) * 30000.0;
					}
					phase += baseCycle;
					if (phase >= 2*M_PI) {
						phase -= 2*M_PI;
					}
				}
				m_interface->write(&data[0], data.size()/m_channelMap.size());
			}
			m_interface->start();
			for (int32_t kkk=0; kkk<100; ++kkk) {
				for (int32_t iii=0; iii<data.size()/m_channelMap.size(); iii++) {
					for (int32_t jjj=0; jjj<m_channelMap.size(); jjj++) {
						data[m_channelMap.size()*iii+jjj] = cos(phase) * 30000.0;
					}
					phase += baseCycle;
					if (phase >= 2*M_PI) {
						phase -= 2*M_PI;
					}
				}
				m_interface->write(&data[0], data.size()/m_channelMap.size());
				// TODO : Add a function to get number of time we need to wait enought time ...
				usleep(15000);
			}
			m_interface->stop();
		}
};

TEST(TestALL, testOutputWrite) {
	std::shared_ptr<airtio::Manager> manager;
	manager = airtio::Manager::create("testApplication");
	
	APPL_INFO("test output (write mode)");
	std::shared_ptr<testOutWrite> process = std::make_shared<testOutWrite>(manager);
	process->run();
	process.reset();
	usleep(500000);
}


class testOutWriteCallback {
	private:
		std::shared_ptr<airtio::Manager> m_manager;
		std::shared_ptr<airtio::Interface> m_interface;
		double m_phase;
	public:
		testOutWriteCallback(std::shared_ptr<airtio::Manager> _manager) :
		  m_manager(_manager),
		  m_phase(0) {
			std::vector<airtalgo::channel> channelMap;
			//Set stereo output:
			channelMap.push_back(airtalgo::channel_frontLeft);
			channelMap.push_back(airtalgo::channel_frontRight);
			m_interface = m_manager->createOutput(48000,
			                                      channelMap,
			                                      airtalgo::format_int16,
			                                      "default",
			                                      "WriteMode+Callback");
			m_interface->setWriteCallback(std::bind(&testOutWriteCallback::onDataNeeded,
			                                        this,
			                                        std::placeholders::_1,
			                                        std::placeholders::_2,
			                                        std::placeholders::_3,
			                                        std::placeholders::_4));
		}
		void onDataNeeded(const std::chrono::system_clock::time_point& _playTime,
		                  const size_t& _nbChunk,
		                  const std::vector<airtalgo::channel>& _map,
		                  enum airtalgo::format _type) {
			if (_type != airtalgo::format_int16) {
				APPL_ERROR("call wrong type ... (need int16_t)");
			}
			std::vector<int16_t> data;
			data.resize(1024*_map.size());
			double baseCycle = 2.0*M_PI/48000.0 * 440.0;
			// start fill buffer
			for (int32_t iii=0; iii<data.size()/_map.size(); iii++) {
				for (int32_t jjj=0; jjj<_map.size(); jjj++) {
					data[_map.size()*iii+jjj] = cos(m_phase) * 30000.0;
				}
				m_phase += baseCycle;
				if (m_phase >= 2*M_PI) {
					m_phase -= 2*M_PI;
				}
			}
			m_interface->write(&data[0], data.size()/_map.size());
		}
		void run() {
			m_interface->start();
			usleep(1000000);
			m_interface->stop();
		}
};

TEST(TestALL, testOutputWriteWithCallback) {
	std::shared_ptr<airtio::Manager> manager;
	manager = airtio::Manager::create("testApplication");
	
	APPL_INFO("test output (write with callback event mode)");
	std::shared_ptr<testOutWriteCallback> process = std::make_shared<testOutWriteCallback>(manager);
	process->run();
	process.reset();
	usleep(500000);
}


class testOutCallback {
	private:
		std::shared_ptr<airtio::Manager> m_manager;
		std::shared_ptr<airtio::Interface> m_interface;
		double m_phase;
	public:
		testOutCallback(std::shared_ptr<airtio::Manager> _manager) :
		  m_manager(_manager),
		  m_phase(0) {
			//Set stereo output:
			std::vector<airtalgo::channel> channelMap;
			channelMap.push_back(airtalgo::channel_frontLeft);
			channelMap.push_back(airtalgo::channel_frontRight);
			m_interface = m_manager->createOutput(48000,
			                                      channelMap,
			                                      airtalgo::format_int16,
			                                      "default",
			                                      "WriteModeCallback");
			// set callback mode ...
			m_interface->setOutputCallback(1024,
			                               std::bind(&testOutCallback::onDataNeeded,
			                                         this,
			                                         std::placeholders::_1,
			                                         std::placeholders::_2,
			                                         std::placeholders::_3,
			                                         std::placeholders::_4,
			                                         std::placeholders::_5));
		}
		void onDataNeeded(const std::chrono::system_clock::time_point& _playTime,
		                  const size_t& _nbChunk,
		                  const std::vector<airtalgo::channel>& _map,
		                  void* _data,
		                  enum airtalgo::format _type) {
			if (_type != airtalgo::format_int16) {
				APPL_ERROR("call wrong type ... (need int16_t)");
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
			m_interface->start();
			// wait 2 second ...
			usleep(2000000);
			m_interface->stop();
		}
};

TEST(TestALL, testOutputCallBack) {
	std::shared_ptr<airtio::Manager> manager;
	manager = airtio::Manager::create("testApplication");
	
	APPL_INFO("test output (callback mode)");
	std::shared_ptr<testOutCallback> process = std::make_shared<testOutCallback>(manager);
	process->run();
	process.reset();
	usleep(500000);
}

/*
class testInRead {
	private:
		std::vector<airtalgo::channel> m_channelMap;
		std::shared_ptr<airtio::Manager> m_manager;
		std::shared_ptr<airtio::Interface> m_interface;
	public:
		testInRead(std::shared_ptr<airtio::Manager> _manager) :
		  m_manager(_manager){
			//Set stereo output:
			m_channelMap.push_back(airtalgo::channel_frontLeft);
			m_channelMap.push_back(airtalgo::channel_frontRight);
			m_interface = m_manager->createInput(48000,
			                                     m_channelMap,
			                                     airtalgo::format_int16,
			                                     "default",
			                                     "WriteMode");
		}
		void run() {
			m_interface->start();
			std::vector<int16_t> data;
			for (int32_t kkk=0; kkk<100; ++kkk) {
				data = m_interface->read(1024);
				int64_t value = 0;
				for (size_t iii=0; iii<data.size(); ++iii) {
					value += std::abs(data[iii]);
				}
				value /= data.size();
				APPL_INFO("Get data ... average=" << int32_t(value));
			}
			m_interface->stop();
		}
};

TEST(TestALL, testInputCallBack) {
	std::shared_ptr<airtio::Manager> manager;
	manager = airtio::Manager::create("testApplication");
	APPL_INFO("test input (callback mode)");
	std::shared_ptr<testInCallback> process = std::make_shared<testInCallback>(manager);
	process->run();
	process.reset();
	usleep(500000);
}
*/

class testInCallback {
	private:
		std::shared_ptr<airtio::Manager> m_manager;
		std::shared_ptr<airtio::Interface> m_interface;
		double m_phase;
	public:
		testInCallback(std::shared_ptr<airtio::Manager> _manager) :
		  m_manager(_manager),
		  m_phase(0) {
			//Set stereo output:
			std::vector<airtalgo::channel> channelMap;
			channelMap.push_back(airtalgo::channel_frontLeft);
			channelMap.push_back(airtalgo::channel_frontRight);
			m_interface = m_manager->createInput(48000,
			                                     channelMap,
			                                     airtalgo::format_int16,
			                                     "default",
			                                     "WriteModeCallback");
			// set callback mode ...
			m_interface->setInputCallback(1024,
			                              std::bind(&testInCallback::onDataReceived,
			                                        this,
			                                        std::placeholders::_1,
			                                        std::placeholders::_2,
			                                        std::placeholders::_3,
			                                        std::placeholders::_4,
			                                        std::placeholders::_5));
		}
		void onDataReceived(const std::chrono::system_clock::time_point& _readTime,
		                    size_t _nbChunk,
		                    const std::vector<airtalgo::channel>& _map,
		                    const void* _data,
		                    enum airtalgo::format _type) {
			if (_type != airtalgo::format_int16) {
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
			m_interface->stop();
		}
};

TEST(TestALL, testInputCallBack) {
	std::shared_ptr<airtio::Manager> manager;
	manager = airtio::Manager::create("testApplication");
	APPL_INFO("test input (callback mode)");
	std::shared_ptr<testInCallback> process = std::make_shared<testInCallback>(manager);
	process->run();
	process.reset();
	usleep(500000);
}


class testOutCallbackType {
	private:
		std::shared_ptr<airtio::Manager> m_manager;
		std::shared_ptr<airtio::Interface> m_interface;
		double m_phase;
		float m_freq;
		int32_t m_nbChannels;
		float m_generateFreq;
		
	public:
		testOutCallbackType(const std::shared_ptr<airtio::Manager>& _manager,
		                    float _freq=48000.0f,
		                    int32_t _nbChannels=2,
		                    airtalgo::format _format=airtalgo::format_int16) :
		  m_manager(_manager),
		  m_phase(0),
		  m_freq(_freq),
		  m_nbChannels(_nbChannels),
		  m_generateFreq(550.0f) {
			//Set stereo output:
			std::vector<airtalgo::channel> channelMap;
			if (m_nbChannels == 1) {
				channelMap.push_back(airtalgo::channel_frontCenter);
			} else if (m_nbChannels == 2) {
				channelMap.push_back(airtalgo::channel_frontLeft);
				channelMap.push_back(airtalgo::channel_frontRight);
			} else if (m_nbChannels == 4) {
				channelMap.push_back(airtalgo::channel_frontLeft);
				channelMap.push_back(airtalgo::channel_frontRight);
				channelMap.push_back(airtalgo::channel_rearLeft);
				channelMap.push_back(airtalgo::channel_rearRight);
			} else {
				APPL_ERROR("Can not generate with channel != 1,2,4");
				return;
			}
			m_interface = m_manager->createOutput(m_freq,
			                                      channelMap,
			                                      _format,
			                                      "default",
			                                      "WriteModeCallbackType");
			// set callback mode ...
			m_interface->setOutputCallback(1024,
			                               std::bind(&testOutCallbackType::onDataNeeded,
			                                         this,
			                                         std::placeholders::_1,
			                                         std::placeholders::_2,
			                                         std::placeholders::_3,
			                                         std::placeholders::_4,
			                                         std::placeholders::_5));
		}
		void onDataNeeded(const std::chrono::system_clock::time_point& _playTime,
		                  const size_t& _nbChunk,
		                  const std::vector<airtalgo::channel>& _map,
		                  void* _data,
		                  enum airtalgo::format _type) {
			APPL_DEBUG("Get data ... " << _type << " map=" << _map << " chunk=" << _nbChunk);
			double baseCycle = 2.0*M_PI/double(m_freq) * double(m_generateFreq);
			if (_type == airtalgo::format_int16) {
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
			} else if (_type == airtalgo::format_int16_on_int32) {
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
			} else if (_type == airtalgo::format_int32) {
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
			} else if (_type == airtalgo::format_float) {
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
			if (m_interface != nullptr) {
				m_interface->start();
				// wait 2 second ...
				usleep(1000000);
				m_interface->stop();
				usleep(100000);
			} else {
				APPL_ERROR("Can not create interface !!!");
			}
		}
};

TEST(TestALL, testResampling) {
	std::shared_ptr<airtio::Manager> manager;
	manager = airtio::Manager::create("testApplication");
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
	for (auto &it : listFreq) {
		std::shared_ptr<testOutCallbackType> process = std::make_shared<testOutCallbackType>(manager, it, 2, airtalgo::format_int16);
		process->run();
		process.reset();
		usleep(500000);
	}
}


TEST(TestALL, testFormat) {
	std::shared_ptr<airtio::Manager> manager;
	manager = airtio::Manager::create("testApplication");
	std::vector<airtalgo::format> listFormat;
	listFormat.push_back(airtalgo::format_int16);
	listFormat.push_back(airtalgo::format_int16_on_int32);
	listFormat.push_back(airtalgo::format_int32);
	listFormat.push_back(airtalgo::format_float);
	for (auto &it : listFormat) {
		std::shared_ptr<testOutCallbackType> process = std::make_shared<testOutCallbackType>(manager, 48000, 2, it);
		process->run();
		process.reset();
		usleep(500000);
	}
}

TEST(TestALL, testChannels) {
	std::shared_ptr<airtio::Manager> manager;
	manager = airtio::Manager::create("testApplication");
	std::vector<int32_t> listChannel;
	listChannel.push_back(1);
	listChannel.push_back(2);
	listChannel.push_back(4);
	for (auto &it : listChannel) {
		std::shared_ptr<testOutCallbackType> process = std::make_shared<testOutCallbackType>(manager, 48000, it, airtalgo::format_int16);
		process->run();
		process.reset();
		usleep(500000);
	}
}


TEST(TestALL, testChannelsFormatResampling) {
	std::shared_ptr<airtio::Manager> manager;
	manager = airtio::Manager::create("testApplication");
	APPL_INFO("test convert flaot to output (callback mode)");
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
	std::vector<airtalgo::format> listFormat;
	listFormat.push_back(airtalgo::format_int16);
	listFormat.push_back(airtalgo::format_int16_on_int32);
	listFormat.push_back(airtalgo::format_int32);
	listFormat.push_back(airtalgo::format_float);
	for (auto &itFreq : listFreq) {
		for (auto &itChannel : listChannel) {
			for (auto &itFormat : listFormat) {
				APPL_INFO("freq=" << itFreq << " channel=" << itChannel << " format=" << getFormatString(itFormat));
				std::shared_ptr<testOutCallbackType> process = std::make_shared<testOutCallbackType>(manager, itFreq, itChannel, itFormat);
				process->run();
				process.reset();
				usleep(500000);
			}
		}
	}
}


int main(int argc, char **argv) {
	// init Google test :
	::testing::InitGoogleTest(&argc, const_cast<char **>(argv));
	// the only one init for etk:
	etk::log::setLevel(etk::log::logLevelNone);
	for (int32_t iii=0; iii<argc ; ++iii) {
		std::string data = argv[iii];
		if (data == "-l0") {
			etk::log::setLevel(etk::log::logLevelNone);
		} else if (data == "-l1") {
			etk::log::setLevel(etk::log::logLevelCritical);
		} else if (data == "-l2") {
			etk::log::setLevel(etk::log::logLevelError);
		} else if (data == "-l3") {
			etk::log::setLevel(etk::log::logLevelWarning);
		} else if (data == "-l4") {
			etk::log::setLevel(etk::log::logLevelInfo);
		} else if (data == "-l5") {
			etk::log::setLevel(etk::log::logLevelDebug);
		} else if (data == "-l6") {
			etk::log::setLevel(etk::log::logLevelVerbose);
		} else if (    data == "-h"
		            || data == "--help") {
			APPL_INFO("Help : ");
			APPL_INFO("    ./xxx [options]");
			APPL_INFO("        -l0: debug None");
			APPL_INFO("        -l1: debug Critical");
			APPL_INFO("        -l2: debug Error");
			APPL_INFO("        -l3: debug Warning");
			APPL_INFO("        -l4: debug Info");
			APPL_INFO("        -l5: debug Debug");
			APPL_INFO("        -l6: debug Verbose");
			APPL_INFO("        -h/--help: this help");
			exit(0);
		}
	}
	etk::setArgZero(argv[0]);
	etk::initDefaultFolder("exml_test");
	return RUN_ALL_TESTS();
}
