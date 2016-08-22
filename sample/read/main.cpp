/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */

#include <audio/river/river.h>
#include <audio/river/Manager.h>
#include <audio/river/Interface.h>
#include <etk/os/FSNode.h>
#include <etk/etk.h>
#include <unistd.h>


static const std::string configurationRiver =
	"{\n"
	"	microphone:{\n"
	"		io:'input',\n"
	"		map-on:{\n"
	"			interface:'auto',\n"
	"			name:'default',\n"
	//"			timestamp-mode:'trigered',\n"
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
                    const std::vector<audio::channel>& _map,
                    etk::FSNode* _outputNode) {
	if (    _format != audio::format_int16
	     && _format != audio::format_float) {
		std::cout << "[ERROR] call wrong type ... (need int16_t.float)" << std::endl;
		return;
	}
	if (_outputNode->fileIsOpen() == false) {
		if (_format != audio::format_int16) {
			// get the curent power of the signal.
			const int16_t* data = static_cast<const int16_t*>(_data);
			int64_t value = 0;
			for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
				value += std::abs(data[iii]);
			}
			value /= (_nbChunk*_map.size());
			std::cout << "Get data ... average=" << int32_t(value) << std::endl;
		} else {
			// get the curent power of the signal.
			const float* data = static_cast<const float*>(_data);
			float value = 0;
			for (size_t iii=0; iii<_nbChunk*_map.size(); ++iii) {
				value += std::abs(data[iii]);
			}
			value /= (_nbChunk*_map.size());
			std::cout << "Get data ... average=" << float(value) << std::endl;
		}
	} else {
		// just write data
		//std::cout << "Get data ... chunks=" << _nbChunk << " time=" << _time << std::endl;
		_outputNode->fileWrite(_data, _map.size()*audio::getFormatBytes(_format), _nbChunk);
	}
}

int main(int _argc, const char **_argv) {
	// the only one init for etk:
	etk::init(_argc, _argv);
	// local parameter:
	std::string configFile;
	std::string ioName="microphone";
	std::string outputFileName = "";
	for (int32_t iii=0; iii<_argc ; ++iii) {
		std::string data = _argv[iii];
		if (    data == "-h"
		     || data == "--help") {
			std::cout << "Help : " << std::endl;
			std::cout << "    --conf=xxx.json Input/output configuration" << std::endl;
			std::cout << "    --io=xxx        name configuration input" << std::endl;
			std::cout << "    --file=yyy.raw  File name to store data" << std::endl;
			exit(0);
		} else if (etk::start_with(data, "--conf=") == true) {
			configFile = std::string(data.begin()+7, data.end());
			std::cout << "Select config: " << configFile << std::endl;
		} else if (etk::start_with(data, "--io=") == true) {
			ioName = std::string(data.begin()+5, data.end());
			std::cout << "Select io: " << ioName << std::endl;
		} else if (etk::start_with(data, "--file=") == true) {
			outputFileName = std::string(data.begin()+7, data.end());
			std::cout << "Select output file name: " << outputFileName << std::endl;
		}
	}
	// initialize river interface
	if (configFile == "") {
		audio::river::initString(configurationRiver);
	} else {
		audio::river::init(configFile);
	}
	// Create the River manager for tha application or part of the application.
	ememory::SharedPtr<audio::river::Manager> manager = audio::river::Manager::create("river_sample_read");
	// create interface:
	ememory::SharedPtr<audio::river::Interface> interface;
	//Get the generic input:
	interface = manager->createInput(48000,
	                                 std::vector<audio::channel>(),
	                                 audio::format_int16,
	                                 ioName);
	if(interface == nullptr) {
		std::cout << "nullptr interface" << std::endl;
		return -1;
	}
	etk::FSNode outputNode;
	// open output file if needed:
	if (outputFileName != "") {
		outputNode.setName(outputFileName);
		outputNode.fileOpenWrite();
	}
	// set callback mode ...
	interface->setInputCallback(std::bind(&onDataReceived,
	                                        std::placeholders::_1,
	                                        std::placeholders::_2,
	                                        std::placeholders::_3,
	                                        std::placeholders::_4,
	                                        std::placeholders::_5,
	                                        std::placeholders::_6,
	                                        &outputNode));
	// start the stream
	interface->start();
	// wait 10 second ...
	sleep(10);
	// stop the stream
	interface->stop();
	// remove interface and manager.
	interface.reset();
	manager.reset();
	// close the output file
	if (outputFileName != "") {
		outputNode.fileClose();
	}
	return 0;
}

