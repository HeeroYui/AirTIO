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

void onDataReceived(const void* _data,
                    const audio::Time& _time,
                    size_t _nbChunk,
                    enum audio::format _format,
                    uint32_t _frequency,
                    const std::vector<audio::channel>& _map) {
	if (_format != audio::format_int16) {
		std::cout << "[ERROR] call wrong type ... (need int16_t)" << std::endl;
	}
	const int16_t* data = static_cast<const int16_t*>(_data);
	int64_t value = 0;
	for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
		value += std::abs(data[iii]);
	}
	value /= (_nbChunk*_map.size());
	std::cout << "Get data ... average=" << int32_t(value) << std::endl;
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
	std11::shared_ptr<audio::river::Manager> manager = audio::river::Manager::create("river_sample_read");
	// create interface:
	std11::shared_ptr<audio::river::Interface> interface;
	//Get the generic input:
	interface = manager->createInput(48000,
	                                 std::vector<audio::channel>(),
	                                 audio::format_int16,
	                                 "microphone");
	if(interface == nullptr) {
		std::cout << "nullptr interface" << std::endl;
		return -1;
	}
	// set callback mode ...
	interface->setInputCallback(std11::bind(&onDataReceived,
	                                        std11::placeholders::_1,
	                                        std11::placeholders::_2,
	                                        std11::placeholders::_3,
	                                        std11::placeholders::_4,
	                                        std11::placeholders::_5,
	                                        std11::placeholders::_6));
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

