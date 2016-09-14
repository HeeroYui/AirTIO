Write stream to Audio output                       {#audio_river_write}
============================

@tableofcontents

Objectifs:                                         {#audio_river_write_objectif}
==========

  - Understand write audio stream

The writing work nearly like the read turoral. Then we will just see what has change.

File configuration:                                {#audio_river_write_config}
===================

@snippet write.cpp audio_river_sample_write_config_file


Create your write interface:                       {#audio_river_write_interface}
============================

Generic code:
@snippet write.cpp audio_river_sample_create_write_interface

Here we create an interface with:
  - The frequency of 48000 Hz.
  - The default Low level definition channel
  - A data interface of 16 bits samples coded in [-32768..32767]
  - Select input interaface name "speaker"


set data callback:                                {#audio_river_write_get_data}
==================

The best way to get data is to instanciate a simple callback.
The callback is called when sample are needed and you have the nbChunk/frequency
to generate the data, otherwise you can generate error in data stream.

@snippet write.cpp audio_river_sample_set_callback

Callback inplementation:                          {#audio_river_write_callback}
========================

Simply declare your function and do what you want inside.

@snippet write.cpp audio_river_sample_callback_implement



Full Sample:                                     {#audio_river_write_full_sample}
============

@snippet write.cpp audio_river_sample_write_all
