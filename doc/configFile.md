River configuration file                           {#audio_river_config_file}
========================

@tableofcontents

Objectifs:                                         {#audio_river_config_file_objectif}
==========

  - Understand the architecture of the configuration file.
  - all that can be done with it.


Basis:                                             {#audio_river_config_file_bases}
======

The river configuration file is a json file. We use @ref ejson_mainpage_what to parse it then we have some writing facilities.


River provide a list a harware interface and virtual interface.


The hardware interface are provided by @ref audio_orchestra_mainpage_what then we will plug on every platform.


The file is simply architecture around a list of object:

```{.json}
{
	"speaker":{
		
	},
	"microphone":{
		
	},
	"mixed-in-out":{
		
	},
}
```

With this config we declare 3 interfaces : speaker, microphone and mixed-in-out.


Harware configuration:                             {#audio_river_config_file_hw_config}
======================

In every interface we need to define some Element:
  - "io" : Can be input/output/... depending of virtual interface...
  - "map-on": An object to configure airtaudio interface.
  - "frequency": 0 to automatic select one. Or the frequency to open harware device
  - "channel-map": List of all channel in the stream:
      * "front-left"
      * "front-center"
      * "front-right"
      * "rear-left"
      * "rear-center"
      * "rear-right"
      * "surround-left",
      * "surround-right",
      * "sub-woofer",
      * "lfe"
  - "type": Fomat to open the stream:
      * "auto": Detect the best type
      * "int8",
      * "int8-on-int16",
      * "int16",
      * "int16-on-int32",
      * "int24",
      * "int32",
      * "int32-on-int64",
      * "int64",
      * "float",
      * "double"
  - "nb-chunk": Number of chunk to open the stream.


Generic configuration file use
==============================

You can specify a generic configuration file foir a user in the path ```~/.local/share/audio-river/config.json```

The default config file is:

```{.json}
{
	microphone:{
		io:"input",
		map-on:{
			interface:"auto",
			name:"default",
		},
		frequency:0,
		channel-map:[
			"front-left", "front-right"
		],
		type:"auto",
		nb-chunk:1024,
		mux-demux-type:"float"
	},
	speaker:{
		io:"output",
		map-on:{
			interface:"alsa",
			name:"default",
		},
		frequency:0,
		channel-map:[
			"front-left", "front-right",
		],
		type:"auto",
		nb-chunk:1024,
		volume-name:"MASTER",
		mux-demux-type:"float"
	}
}
```

If the pplication start with no name it try to load this file and if it fail it load the internalversion of a basic file

