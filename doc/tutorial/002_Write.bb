
=== Objectif ===
:** Understand write audio stream

=== sample source: ===
	[[http://github.com/HeeroYui/river.git/sample/write/ | sample source]]

=== Bases: ===

The writing work nearly like the read turoral. Then we will just see what has change.

==== File configuration: ====

[code style=c++]
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
[/code]

==== Create your write interface: ====

[code style=c++]
// create interface:
std11::shared_ptr<river::Interface> interface;
//Get the generic input:
interface = manager->createOutput(48000,
                                  std::vector<audio::channel>(),
                                  audio::format_int16,
                                  "speaker");
[/code]

Here we create an interface with:
:** The frequency of 48000 Hz.
:** The default Low level definition channel
:** A data interface of 16 bits samples coded in [-32768..32767]
:** Select input interaface name "speaker"


==== write datas: ====

The best way to get data is to instanciate a simple callback.
The callback is called when sample are needed and you have the nbChunk/frequency
to generate the data, otherwise you can generate error in data stream.


[code style=c++]
// set callback mode ...
interface->setOutputCallback(std11::bind(&onDataNeeded,
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
void onDataNeeded(void* _data,
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
