
=== Objectif ===
:** Understand the architecture of the configuration file.
:** all that can be done with it.


=== Basis: ===

The river configuration file is a json file. We use [lib[ejson | e-json library]] to parse it then we have some writing facilities.


River provide a list a harware interface and virtual interface.


The hardware interface are provided by [lib[airtaudio | AirTAudio library]] then we will plug on every platform.


The file is simply architecture around a list of object:

[code style=json]
{
	"speaker":{
		
	},
	"microphone":{
		
	},
	"mixed-in-out":{
		
	},
}
[/code]

With this config we declare 3 interfaces : speaker, microphone and mixed-in-out.


=== Harware configuration: ===

In every interface we need to define some Element:
:** "io" :
:: Can be input/output/... depending of virtual interface...
:** "map-on": An object to configure airtaudio interface.
:** "frequency": 0 to automatic select one. Or the frequency to open harware device
:** "channel-map": List of all channel in the stream:
::** "front-left"
::** "front-center"
::** "front-right"
::** "rear-left"
::** "rear-center"
::** "rear-right"
::** "surround-left",
::** "surround-right",
::** "sub-woofer",
::** "lfe"
:** "type": Fomat to open the stream:
::** "auto": Detect the best type
::** "int8",
::** "int8-on-int16",
::** "int16",
::** "int16-on-int32",
::** "int24",
::** "int32",
::** "int32-on-int64",
::** "int64",
::** "float",
::** "double"
:** "nb-chunk": Number of chunk to open the stream.



