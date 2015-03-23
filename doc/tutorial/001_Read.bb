
=== Objectif ===
:** Understand basis of river
:** Create a simple recording interface that print the average of sample absolute value.

=== sample source: ===
	[[http://github.com/HeeroYui/river.git/sample/read/ | sample source]]

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

[note]
This json is parsed by the [lib[ejson | e-json library]] it containe some update like:
:** Optionnal " in the name of element.
:** The possibilities to remplace " with '.
[/note]


==== Get the river interface manager: ====

An application can have many interface and only one Manager, And a process can contain many application.

Then, we will get the first application manager handle.

[code style=c++]
// Create the River manager for tha application or part of the application.
std11::shared_ptr<river::Manager> manager = river::Manager::create("river_sample_read");
[/code]

[note]
You can get back the application handle when you create a new one with the same name.
[/note]

==== Create your read interface: ====

[code style=c++]
// create interface:
std11::shared_ptr<river::Interface> interface;
//Get the generic input:
interface = manager->createInput(48000,
                                 std::vector<audio::channel>(),
                                 audio::format_int16,
                                 "microphone");
[/code]

Here we create an interface with:
:** The frequency of 48000 Hz.
:** The default Low level definition channel
:** A data interface of 16 bits samples coded in [-32768..32767]
:** Select input interaface name "microphone"


==== Get datas: ====

The best way to get data is to instanciate a simple callback.
The callback is called when sample arrive and you have the nbChunk/frequency
to process the data, otherwise you can generate error in data stream.


[code style=c++]
// set callback mode ...
interface->setInputCallback(std11::bind(&onDataReceived,
                                        std11::placeholders::_1,
                                        std11::placeholders::_2,
                                        std11::placeholders::_3,
                                        std11::placeholders::_4,
                                        std11::placeholders::_5,
                                        std11::placeholders::_6));
[/code]

==== Callback inplementation: ====

Simply declare your function and do what you want inside.

[code style=c++]
void onDataReceived(const void* _data,
                    const std11::chrono::system_clock::time_point& _time,
                    size_t _nbChunk,
                    enum audio::format _format,
                    uint32_t _frequency,
                    const std::vector<audio::channel>& _map) {
	if (_format == audio::format_int16) {
		// stuff here
	}
}
[/code]

==== start and stop: ====

[code style=c++]
// start the stream
interface->start();
// wait 10 second ...
sleep(10);
// stop the stream
interface->stop();
[/code]

==== Remove interfaces: ====

[code style=c++]
// remove interface and manager.
interface.reset();
manager.reset();
[/code]
