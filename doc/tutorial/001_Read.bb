
=== Objectif ===
:** Understand basis of river
:** Create a simple recording interface that print the average of sample absolute value.

=== sample source: ===
	[[http:://github.com/HeeroYui/river.git/sample/read/ | sample source]]

=== Bases: ===

When you will create an application based on the river audio interface you need :

==== Include: ====

Include manager and interface node

[code style=c++]
#include <river/river.h>
#include <river/Manager.h>
#include <river/Interface.h>
[/code]

==== Initilize the River library: ====

We first need to initialize etk sub library (needed to select the log level of sub-libraries and file access abstraction
[code style=c++]
	// the only one init for etk:
	etk::init(_argc, _argv);
[/code]

Now we will initilaize the river library.
To do this We have 2 posibilities:
:** With a file:
[code style=c++]
	// initialize river interface
	river::init("DATA:configFileName.json");
[/code]
:** With a json string:
[code style=c++]
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
	// initialize river interface
	river::initString(configurationRiver);
[/code]

For the example we select the second solution (faster to implement example and resource at the same position.

river::init / river::initString must be called only one time for all the application, this represent the hardware configuration.
It is Nearly not dynamic

To understand the configuration file Please see [tutorial[004_ConfigurationFile | Configuration file]]



	// Create the River manager for tha application or part of the application.
	std11::shared_ptr<river::Manager> manager = river::Manager::create("river_sample_read");
	// create interface:
	std11::shared_ptr<river::Interface> interface;
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