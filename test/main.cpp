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

#if 0
class testOutWrite {
	private:
		std::vector<airtalgo::channel> m_channelMap;
		std::shared_ptr<airtio::Manager> m_manager;
		std::shared_ptr<airtio::Interface> m_interface;
	public:
		testOutWrite(std::shared_ptr<airtio::Manager> _manager) :
		  m_manager(_manager){
			//Set stereo output:
			m_channelMap.push_back(airtalgo::channel_frontLeft);
			m_channelMap.push_back(airtalgo::channel_frontRight);
			m_interface = m_manager->createOutput(48000,
			                                      m_channelMap,
			                                      airtalgo::format_int16,
			                                      "default",
			                                      "WriteMode");
		}
		~testOutWrite() {
			
		}
		
		void run() {
			m_interface->start();
			double phase=0;
			std::vector<int16_t> data;
			data.resize(1024*m_channelMap.size());
			double baseCycle = 2.0*M_PI/(double)48000 * (double)440;
			for (int32_t kkk=0; kkk<100; ++kkk) {
				for (int32_t iii=0; iii<data.size()/m_channelMap.size(); iii++) {
					for (int32_t jjj=0; jjj<m_channelMap.size(); jjj++) {
						data[m_channelMap.size()*iii+jjj] = cos(phase) * 30000;
					}
					phase += baseCycle;
					if (phase >= 2*M_PI) {
						phase -= 2*M_PI;
					}
				}
				APPL_INFO("send data");
				m_interface->write(data);
			}
			m_interface->stop();
		}
};
#endif

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
			                                         std::placeholders::_5),
			                               airtalgo::formatDataTypeInt16);
		}
		
		~testOutCallback() {
			
		}
		
		void onDataNeeded(const std::chrono::system_clock::time_point& _playTime,
		                  const size_t& _nbChunk,
		                  const std::vector<airtalgo::channel>& _map,
		                  void* _data,
		                  enum airtalgo::formatDataType& _type) {
			if (_type != airtalgo::formatDataTypeInt16) {
				APPL_ERROR("call wrong type ...");
			}
			int16_t* data = static_cast<int16_t*>(_data);
			double baseCycle = 2.0*M_PI/(double)48000 * (double)550;
			APPL_INFO("Get data ...");
			for (int32_t iii=0; iii<_nbChunk; iii++) {
				for (int32_t jjj=0; jjj<_map.size(); jjj++) {
					data[_map.size()*iii+jjj] = cos(m_phase) * 30000;
				}
				m_phase += baseCycle;
				if (m_phase >= 2*M_PI) {
					m_phase -= 2*M_PI;
				}
			}
			/*
			std::vector<int16_t> data;
			data.resize(_nbChunk*_map.size());
			double baseCycle = 2.0*M_PI/(double)48000 * (double)550;
			APPL_INFO("Get data ...");
			
			for (int32_t iii=0; iii<data.size()/_map.size(); iii++) {
				for (int32_t jjj=0; jjj<_map.size(); jjj++) {
					data[_map.size()*iii+jjj] = cos(m_phase) * 30000;
				}
				m_phase += baseCycle;
				if (m_phase >= 2*M_PI) {
					m_phase -= 2*M_PI;
				}
			}
			return data;
			*/
		}
		
		void run() {
			m_interface->start();
			// wait 2 second ...
			usleep(2000000);
			m_interface->stop();
		}
};

#if 0
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
		~testInRead() {
			
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
			m_interface->setInputCallbackInt16(1024, std::bind(&testInCallback::onDataReceived, this, _1, _2, _3, _4));
		}
		
		~testInCallback() {
			
		}
		
		void onDataReceived(const std::chrono::system_clock::time_point& _playTime,
		                    const size_t& _nbChunk,
		                    const std::vector<airtalgo::channel>& _map,
		                    const std::vector<int16_t>& _data) {
			int64_t value = 0;
			for (size_t iii=0; iii<_data.size(); ++iii) {
				value += std::abs(_data[iii]);
			}
			value /= _data.size();
			APPL_INFO("Get data ... average=" << int32_t(value));
		}
		
		void run() {
			m_interface->start();
			// wait 2 second ...
			usleep(2000000);
			m_interface->stop();
		}
};


class testOutCallbackFloat {
	private:
		std::shared_ptr<airtio::Manager> m_manager;
		std::shared_ptr<airtio::Interface> m_interface;
		double m_phase;
		float m_freq;
		int32_t m_nbChannels;
		float m_generateFreq;
		
	public:
		testOutCallbackFloat(std::shared_ptr<airtio::Manager> _manager,
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
			switch (_format) {
				case airtalgo::format_int16:
					m_interface = m_manager->createOutput(m_freq,
					                                      channelMap,
					                                      _format,
					                                      "default",
					                                      "WriteModeCallbackI16");
					// set callback mode ...
					APPL_ERROR("Set callback");
					m_interface->setOutputCallbackInt16(1024, std::bind(&testOutCallbackFloat::onDataNeededI16, this, _1, _2, _3));
					break;
				case airtalgo::format_int16_on_int32:
					m_interface = m_manager->createOutput(m_freq,
					                                      channelMap,
					                                      _format,
					                                      "default",
					                                      "WriteModeCallbackI16onI32");
					// set callback mode ...
					m_interface->setOutputCallbackInt32(1024, std::bind(&testOutCallbackFloat::onDataNeededI16_I32, this, _1, _2, _3));
					break;
				case airtalgo::format_int32:
					m_interface = m_manager->createOutput(m_freq,
					                                      channelMap,
					                                      _format,
					                                      "default",
					                                      "WriteModeCallbackI32");
					// set callback mode ...
					m_interface->setOutputCallbackInt32(1024, std::bind(&testOutCallbackFloat::onDataNeededI32, this, _1, _2, _3));
					break;
				case airtalgo::format_float:
					m_interface = m_manager->createOutput(m_freq,
					                                      channelMap,
					                                      _format,
					                                      "default",
					                                      "WriteModeCallbackFloat");
					// set callback mode ...
					m_interface->setOutputCallbackFloat(1024, std::bind(&testOutCallbackFloat::onDataNeededFloat, this, _1, _2, _3));
					break;
			}
		}
		
		~testOutCallbackFloat() {
			
		}
		
		std::vector<int16_t> onDataNeededI16(const std::chrono::system_clock::time_point& _playTime,
		                                     const size_t& _nbChunk,
		                                     const std::vector<airtalgo::channel>& _map) {
			std::vector<int16_t> data;
			data.resize(_nbChunk*_map.size());
			double baseCycle = 2.0*M_PI/(double)m_freq * (double)m_generateFreq;
			APPL_INFO("Get data ... " << _map.size());
			
			for (int32_t iii=0; iii<data.size()/_map.size(); iii++) {
				for (int32_t jjj=0; jjj<_map.size(); jjj++) {
					data[_map.size()*iii+jjj] = cos(m_phase) * (double)INT16_MAX;
				}
				m_phase += baseCycle;
				if (m_phase >= 2*M_PI) {
					m_phase -= 2*M_PI;
				}
			}
			return data;
		}
		
		std::vector<int32_t> onDataNeededI16_I32(const std::chrono::system_clock::time_point& _playTime,
		                                         const size_t& _nbChunk,
		                                         const std::vector<airtalgo::channel>& _map) {
			std::vector<int32_t> data;
			data.resize(_nbChunk*_map.size());
			double baseCycle = 2.0*M_PI/(double)m_freq * (double)m_generateFreq;
			APPL_VERBOSE("Get data ...");
			
			for (int32_t iii=0; iii<data.size()/_map.size(); iii++) {
				for (int32_t jjj=0; jjj<_map.size(); jjj++) {
					data[_map.size()*iii+jjj] = cos(m_phase) * (double)INT16_MAX;
				}
				m_phase += baseCycle;
				if (m_phase >= 2*M_PI) {
					m_phase -= 2*M_PI;
				}
			}
			return data;
		}
		
		std::vector<int32_t> onDataNeededI32(const std::chrono::system_clock::time_point& _playTime,
		                                         const size_t& _nbChunk,
		                                         const std::vector<airtalgo::channel>& _map) {
			std::vector<int32_t> data;
			data.resize(_nbChunk*_map.size());
			double baseCycle = 2.0*M_PI/(double)m_freq * (double)m_generateFreq;
			APPL_VERBOSE("Get data ...");
			
			for (int32_t iii=0; iii<data.size()/_map.size(); iii++) {
				for (int32_t jjj=0; jjj<_map.size(); jjj++) {
					data[_map.size()*iii+jjj] = cos(m_phase) * (double)INT32_MAX;
				}
				m_phase += baseCycle;
				if (m_phase >= 2*M_PI) {
					m_phase -= 2*M_PI;
				}
			}
			return data;
		}
		
		
		
		std::vector<float> onDataNeededFloat(const std::chrono::system_clock::time_point& _playTime,
		                                     const size_t& _nbChunk,
		                                     const std::vector<airtalgo::channel>& _map) {
			std::vector<float> data;
			data.resize(_nbChunk*_map.size());
			double baseCycle = 2.0*M_PI/(double)m_freq * (double)m_generateFreq;
			APPL_VERBOSE("Get data ...");
			
			for (int32_t iii=0; iii<data.size()/_map.size(); iii++) {
				for (int32_t jjj=0; jjj<_map.size(); jjj++) {
					data[_map.size()*iii+jjj] = cos(m_phase);
				}
				m_phase += baseCycle;
				if (m_phase >= 2*M_PI) {
					m_phase -= 2*M_PI;
				}
			}
			return data;
		}
		
		void run() {
			if (m_interface != NULL) {
				m_interface->start();
				// wait 2 second ...
				usleep(1000000);
				m_interface->stop();
			} else {
				APPL_ERROR("Can not create interface !!!");
			}
		}
};
#endif


TEST(TestALL, testOutputCallBack) {
	std::shared_ptr<airtio::Manager> manager;
	manager = airtio::Manager::create("testApplication");
	
	APPL_INFO("test output (callback mode)");
	std::shared_ptr<testOutCallback> process = std::make_shared<testOutCallback>(manager);
	process->run();
	process.reset();
	usleep(500000);
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
	
	#if 0
	APPL_INFO("test output (Write mode)");
	{
		std::shared_ptr<testOutWrite> process = std::make_shared<testOutWrite>(manager);
		process->run();
		process.reset();
	}
	usleep(500000);
	#endif
	
	#if 0
	APPL_INFO("test input (Read mode)");
	{
		std::shared_ptr<testInRead> process = std::make_shared<testInRead>(manager);
		process->run();
		process.reset();
	}
	usleep(500000);
	#endif
	
	#if 0
	APPL_INFO("test input (callback mode)");
	{
		std::shared_ptr<testInCallback> process = std::make_shared<testInCallback>(manager);
		process->run();
		process.reset();
	}
	#endif
	
	
	#if 0
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
	for (int32_t iii=0; iii<listFreq.size(); ++iii) {
		for (int32_t jjj=0; jjj<listChannel.size(); ++jjj) {
			for (std::vector<airtalgo::format>::iterator formatIt = listFormat.begin(); formatIt != listFormat.end(); ++formatIt) {
				float freq = listFreq[iii];
				int32_t channel = listChannel[jjj];
				APPL_INFO("freq=" << freq << " channel=" << channel << " format=" << getFormatString(*formatIt));
				std::shared_ptr<testOutCallbackFloat> process = std::make_shared<testOutCallbackFloat>(manager, freq, channel, *formatIt);
				process->run();
				process.reset();
				usleep(500000);
			}
		}
	}
	#endif
}

