Read stream form Audio input                       {#audio_river_read}
============================

@tableofcontents

Objectifs:                                          {#audio_river_read_objectif}
==========

  - Understand basis of river
  - Create a simple recording interface that print the average of sample absolute value.


When you will create an application based on the river audio interface you need :

Include:                                            {#audio_river_read_include}
========

Include manager and interface node
@snippet read.cpp audio_river_sample_include

Initilize the River library:                        {#audio_river_read_init}
============================

We first need to initialize etk sub library (needed to select the log level of sub-libraries and file access abstraction
@snippet read.cpp audio_river_sample_init

Now we will initilaize the river library.
To do this We have 2 posibilities:
With a file:
------------

```{.cpp}
	// initialize river interface
	river::init("DATA:///configFileName.json");
```

With a json string:
-------------------

@snippet read.cpp audio_river_sample_read_config_file

```{.cpp}
// initialize river interface
river::initString(configurationRiver);
```

For the example we select the second solution (faster to implement example and resource at the same position.

river::init / river::initString must be called only one time for all the application, this represent the hardware configuration.
It is NOT dynamic

To understand the configuration file Please see @ref audio_river_config_file

This json is parsed by the @ref {#ejson_mainpage_what} it contain some update like:
  - Optionnal " in the name of element.
  - The possibilities to remplace " with '.



Get the river interface manager:                        {#audio_river_read_river_interface}
================================

An application can have many interface and only one Manager. And a process can contain many application.

Then, we will get the first application manager handle.
@snippet read.cpp audio_river_sample_get_interface

*Note:* You can get back the application handle when you create a new one with the same name.

Create your read interface:                            {#audio_river_read_river_read_interface}
===========================

Generic code:
@snippet read.cpp audio_river_sample_create_read_interface

Here we create an interface with:
  - The frequency of 48000 Hz.
  - The default Low level definition channel
  - A data interface of 16 bits samples coded in [-32768..32767]
  - Select input interaface name "microphone"


set data callback:                                     {#audio_river_read_get_data}
==================

The best way to get data is to instanciate a simple callback. 
The callback is called when sample arrive and you have the nbChunk/frequency
to process the data, otherwise you can generate error in data stream.

@snippet read.cpp audio_river_sample_set_callback

Callback inplementation:                               {#audio_river_read_callback}
========================

Simply declare your function and do what you want inside.

@snippet read.cpp audio_river_sample_callback_implement

start and stop the stream:                             {#audio_river_read_start_stop}
==========================

@snippet read.cpp audio_river_sample_read_start_stop

Remove interfaces:                                     {#audio_river_read_reset}
==================

@snippet read.cpp audio_river_sample_read_reset




Full Sample:                                     {#audio_river_read_full_sample}
============

@snippet read.cpp audio_river_sample_read_all
