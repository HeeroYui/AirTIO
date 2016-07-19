/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <audio/river/river.h>
#include <audio/river/Manager.h>
#include <audio/river/Interface.h>
#include <etk/etk.h>
#include <unistd.h>


static const std::string configurationRiver =
		"{\n"
		"	speaker:{\n"
		"		io:'output',\n"
		"		map-on:{\n"
		"			interface:'alsa',\n"
		"			name:'hw:2,0',\n"
		"		},\n"
		"		frequency:0,\n"
		//"		channel-map:['front-left', 'front-right', 'rear-left', 'rear-right'],\n"
		"		channel-map:['front-left', 'front-right'],\n"
		"		type:'int32',\n"
		"		nb-chunk:1024,\n"
		"		volume-name:'MASTER'\n"
		"	}\n"
		"}\n";

static const int32_t nbChannelMax=8;

void onDataNeeded(void* _data,
                  const audio::Time& _time,
                  size_t _nbChunk,
                  enum audio::format _format,
                  uint32_t _frequency,
                  const std::vector<audio::channel>& _map) {
	static double phase[8] = {0,0,0,0,0,0,0,0};
	
	if (_format != audio::format_int16) {
		std::cout << "[ERROR] call wrong type ... (need int16_t)" << std::endl;
	}
	//std::cout << "Map " << _map << std::endl;
	int16_t* data = static_cast<int16_t*>(_data);
	double baseCycle = 2.0*M_PI/double(48000) * double(440);
	for (int32_t iii=0; iii<_nbChunk; iii++) {
		for (int32_t jjj=0; jjj<_map.size(); jjj++) {
			data[_map.size()*iii+jjj] = cos(phase[jjj]) * 30000;
			phase[jjj] += baseCycle*jjj;
			if (phase[jjj] >= 2*M_PI) {
				phase[jjj] -= 2*M_PI;
			}
		}
	}
}

int main(int _argc, const char **_argv) {
	// the only one init for etk:
	etk::init(_argc, _argv);
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (    data == "-h"
		     || data == "--help") {
			std::cout << "Help : " << std::endl;
			std::cout << "    ./xxx ---" << std::endl;
			exit(0);
		}
	}
	// initialize river interface
	audio::river::initString(configurationRiver);
	// Create the River manager for tha application or part of the application.
	ememory::SharedPtr<audio::river::Manager> manager = audio::river::Manager::create("river_sample_read");
	// create interface:
	ememory::SharedPtr<audio::river::Interface> interface;
	//Get the generic input:
	interface = manager->createOutput(48000,
	                                  std::vector<audio::channel>(),
	                                  audio::format_int16,
	                                  "speaker");
	if(interface == nullptr) {
		std::cout << "nullptr interface" << std::endl;
		return -1;
	}
	// set callback mode ...
	interface->setOutputCallback(std::bind(&onDataNeeded,
	                                         std::placeholders::_1,
	                                         std::placeholders::_2,
	                                         std::placeholders::_3,
	                                         std::placeholders::_4,
	                                         std::placeholders::_5,
	                                         std::placeholders::_6));
	// start the stream
	interface->start();
	// wait 10 second ...
	sleep(10);
	// stop the stream
	interface->stop();
	// remove interface and manager.
	interface.reset();
	manager.reset();
	return 0;
}

