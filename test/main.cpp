#include "ros/ros.h"
#include <audio_base/Manager.hpp>
#include <audio_base/Interface.hpp>

#include <sstream>


class testOutWrite {
	private:
		std::vector<audio_algo_core::channel> m_channelMap;
		boost::shared_ptr<audio_base::Manager> m_manager;
		boost::shared_ptr<audio_base::Interface> m_interface;
	public:
		testOutWrite(boost::shared_ptr<audio_base::Manager> _manager) :
		  m_manager(_manager){
			//Set stereo output:
			m_channelMap.push_back(audio_algo_core::channel_frontLeft);
			m_channelMap.push_back(audio_algo_core::channel_frontRight);
			m_interface = m_manager->createOutput(48000,
			                                      m_channelMap,
			                                      audio_algo_core::format_int16,
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
				ROS_INFO("send data");
				m_interface->write(data);
			}
			m_interface->stop();
		}
};


class testOutCallback {
	private:
		boost::shared_ptr<audio_base::Manager> m_manager;
		boost::shared_ptr<audio_base::Interface> m_interface;
		double m_phase;
	public:
		testOutCallback(boost::shared_ptr<audio_base::Manager> _manager) :
		  m_manager(_manager),
		  m_phase(0) {
			//Set stereo output:
			std::vector<audio_algo_core::channel> channelMap;
			channelMap.push_back(audio_algo_core::channel_frontLeft);
			channelMap.push_back(audio_algo_core::channel_frontRight);
			m_interface = m_manager->createOutput(48000,
			                                      channelMap,
			                                      audio_algo_core::format_int16,
			                                      "default",
			                                      "WriteModeCallback");
			// set callback mode ...
			m_interface->setOutputCallbackInt16(1024, boost::bind(&testOutCallback::onDataNeeded, this, _1, _2, _3));
		}
		
		~testOutCallback() {
			
		}
		
		std::vector<int16_t> onDataNeeded(const boost::chrono::system_clock::time_point& _playTime,
		                                  const size_t& _nbChunk,
		                                  const std::vector<audio_algo_core::channel>& _map) {
			std::vector<int16_t> data;
			data.resize(_nbChunk*_map.size());
			double baseCycle = 2.0*M_PI/(double)48000 * (double)550;
			ROS_INFO("Get data ...");
			
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
		}
		
		void run() {
			m_interface->start();
			// wait 2 second ...
			usleep(2000000);
			m_interface->stop();
		}
};


class testInRead {
	private:
		std::vector<audio_algo_core::channel> m_channelMap;
		boost::shared_ptr<audio_base::Manager> m_manager;
		boost::shared_ptr<audio_base::Interface> m_interface;
	public:
		testInRead(boost::shared_ptr<audio_base::Manager> _manager) :
		  m_manager(_manager){
			//Set stereo output:
			m_channelMap.push_back(audio_algo_core::channel_frontLeft);
			m_channelMap.push_back(audio_algo_core::channel_frontRight);
			m_interface = m_manager->createInput(48000,
			                                     m_channelMap,
			                                     audio_algo_core::format_int16,
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
				ROS_INFO("Get data ... average = %d", static_cast<int32_t>(value));
			}
			m_interface->stop();
		}
};


class testInCallback {
	private:
		boost::shared_ptr<audio_base::Manager> m_manager;
		boost::shared_ptr<audio_base::Interface> m_interface;
		double m_phase;
	public:
		testInCallback(boost::shared_ptr<audio_base::Manager> _manager) :
		  m_manager(_manager),
		  m_phase(0) {
			//Set stereo output:
			std::vector<audio_algo_core::channel> channelMap;
			channelMap.push_back(audio_algo_core::channel_frontLeft);
			channelMap.push_back(audio_algo_core::channel_frontRight);
			m_interface = m_manager->createInput(48000,
			                                     channelMap,
			                                     audio_algo_core::format_int16,
			                                     "default",
			                                     "WriteModeCallback");
			// set callback mode ...
			m_interface->setInputCallbackInt16(1024, boost::bind(&testInCallback::onDataReceived, this, _1, _2, _3, _4));
		}
		
		~testInCallback() {
			
		}
		
		void onDataReceived(const boost::chrono::system_clock::time_point& _playTime,
		                    const size_t& _nbChunk,
		                    const std::vector<audio_algo_core::channel>& _map,
		                    const std::vector<int16_t>& _data) {
			int64_t value = 0;
			for (size_t iii=0; iii<_data.size(); ++iii) {
				value += std::abs(_data[iii]);
			}
			value /= _data.size();
			ROS_INFO("Get data ... average = %d", static_cast<int32_t>(value));
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
		boost::shared_ptr<audio_base::Manager> m_manager;
		boost::shared_ptr<audio_base::Interface> m_interface;
		double m_phase;
		float m_freq;
		int32_t m_nbChannels;
		float m_generateFreq;
		
	public:
		testOutCallbackFloat(boost::shared_ptr<audio_base::Manager> _manager,
		                     float _freq=48000.0f,
		                     int32_t _nbChannels=2,
		                     audio_algo_core::format _format=audio_algo_core::format_int16) :
		  m_manager(_manager),
		  m_phase(0),
		  m_freq(_freq),
		  m_nbChannels(_nbChannels),
		  m_generateFreq(550.0f) {
			//Set stereo output:
			std::vector<audio_algo_core::channel> channelMap;
			if (m_nbChannels == 1) {
				channelMap.push_back(audio_algo_core::channel_frontCenter);
			} else if (m_nbChannels == 2) {
				channelMap.push_back(audio_algo_core::channel_frontLeft);
				channelMap.push_back(audio_algo_core::channel_frontRight);
			} else if (m_nbChannels == 4) {
				channelMap.push_back(audio_algo_core::channel_frontLeft);
				channelMap.push_back(audio_algo_core::channel_frontRight);
				channelMap.push_back(audio_algo_core::channel_rearLeft);
				channelMap.push_back(audio_algo_core::channel_rearRight);
			} else {
				ROS_ERROR("Can not generate with channel != 1,2,4");
				return;
			}
			switch (_format) {
				case audio_algo_core::format_int16:
					m_interface = m_manager->createOutput(m_freq,
					                                      channelMap,
					                                      _format,
					                                      "default",
					                                      "WriteModeCallbackI16");
					// set callback mode ...
					ROS_ERROR("Set callback");
					m_interface->setOutputCallbackInt16(1024, boost::bind(&testOutCallbackFloat::onDataNeededI16, this, _1, _2, _3));
					break;
				case audio_algo_core::format_int16_on_int32:
					m_interface = m_manager->createOutput(m_freq,
					                                      channelMap,
					                                      _format,
					                                      "default",
					                                      "WriteModeCallbackI16onI32");
					// set callback mode ...
					m_interface->setOutputCallbackInt32(1024, boost::bind(&testOutCallbackFloat::onDataNeededI16_I32, this, _1, _2, _3));
					break;
				case audio_algo_core::format_int32:
					m_interface = m_manager->createOutput(m_freq,
					                                      channelMap,
					                                      _format,
					                                      "default",
					                                      "WriteModeCallbackI32");
					// set callback mode ...
					m_interface->setOutputCallbackInt32(1024, boost::bind(&testOutCallbackFloat::onDataNeededI32, this, _1, _2, _3));
					break;
				case audio_algo_core::format_float:
					m_interface = m_manager->createOutput(m_freq,
					                                      channelMap,
					                                      _format,
					                                      "default",
					                                      "WriteModeCallbackFloat");
					// set callback mode ...
					m_interface->setOutputCallbackFloat(1024, boost::bind(&testOutCallbackFloat::onDataNeededFloat, this, _1, _2, _3));
					break;
			}
		}
		
		~testOutCallbackFloat() {
			
		}
		
		std::vector<int16_t> onDataNeededI16(const boost::chrono::system_clock::time_point& _playTime,
		                                     const size_t& _nbChunk,
		                                     const std::vector<audio_algo_core::channel>& _map) {
			std::vector<int16_t> data;
			data.resize(_nbChunk*_map.size());
			double baseCycle = 2.0*M_PI/(double)m_freq * (double)m_generateFreq;
			ROS_INFO("Get data ... %ld", _map.size());
			
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
		
		std::vector<int32_t> onDataNeededI16_I32(const boost::chrono::system_clock::time_point& _playTime,
		                                         const size_t& _nbChunk,
		                                         const std::vector<audio_algo_core::channel>& _map) {
			std::vector<int32_t> data;
			data.resize(_nbChunk*_map.size());
			double baseCycle = 2.0*M_PI/(double)m_freq * (double)m_generateFreq;
			//ROS_INFO("Get data ...");
			
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
		
		std::vector<int32_t> onDataNeededI32(const boost::chrono::system_clock::time_point& _playTime,
		                                         const size_t& _nbChunk,
		                                         const std::vector<audio_algo_core::channel>& _map) {
			std::vector<int32_t> data;
			data.resize(_nbChunk*_map.size());
			double baseCycle = 2.0*M_PI/(double)m_freq * (double)m_generateFreq;
			//ROS_INFO("Get data ...");
			
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
		
		
		
		std::vector<float> onDataNeededFloat(const boost::chrono::system_clock::time_point& _playTime,
		                                     const size_t& _nbChunk,
		                                     const std::vector<audio_algo_core::channel>& _map) {
			std::vector<float> data;
			data.resize(_nbChunk*_map.size());
			double baseCycle = 2.0*M_PI/(double)m_freq * (double)m_generateFreq;
			//ROS_INFO("Get data ...");
			
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
				ROS_ERROR("Can not create interface !!!");
			}
		}
};





int main(int argc, char **argv) {
	boost::shared_ptr<audio_base::Manager> manager;
	manager = audio_base::Manager::create("testApplication");
	#if 0
	ROS_INFO("test output (Write mode)");
	{
		boost::shared_ptr<testOutWrite> process = boost::make_shared<testOutWrite>(manager);
		process->run();
		process.reset();
	}
	usleep(500000);
	#endif
	
	#if 0
	ROS_INFO("test output (callback mode)");
	{
		boost::shared_ptr<testOutCallback> process = boost::make_shared<testOutCallback>(manager);
		process->run();
		process.reset();
	}
	usleep(500000);
	#endif
	
	#if 0
	ROS_INFO("test input (Read mode)");
	{
		boost::shared_ptr<testInRead> process = boost::make_shared<testInRead>(manager);
		process->run();
		process.reset();
	}
	usleep(500000);
	#endif
	
	#if 0
	ROS_INFO("test input (callback mode)");
	{
		boost::shared_ptr<testInCallback> process = boost::make_shared<testInCallback>(manager);
		process->run();
		process.reset();
	}
	#endif
	
	
	#if 1
	ROS_INFO("test convert flaot to output (callback mode)");
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
	std::vector<audio_algo_core::format> listFormat;
	listFormat.push_back(audio_algo_core::format_int16);
	listFormat.push_back(audio_algo_core::format_int16_on_int32);
	listFormat.push_back(audio_algo_core::format_int32);
	listFormat.push_back(audio_algo_core::format_float);
	for (int32_t iii=0; iii<listFreq.size(); ++iii) {
		for (int32_t jjj=0; jjj<listChannel.size(); ++jjj) {
			for (std::vector<audio_algo_core::format>::iterator formatIt = listFormat.begin(); formatIt != listFormat.end(); ++formatIt) {
				float freq = listFreq[iii];
				int32_t channel = listChannel[jjj];
				ROS_INFO("freq = %f channel=%d format=%s", freq, channel, getFormatString(*formatIt).c_str());
				boost::shared_ptr<testOutCallbackFloat> process = boost::make_shared<testOutCallbackFloat>(manager, freq, channel, *formatIt);
				process->run();
				process.reset();
				usleep(500000);
			}
		}
	}
	#endif
	ROS_INFO("TEST ended");
	
	return 0;
}

