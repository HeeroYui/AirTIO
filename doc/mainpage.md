AUDIO-RIVER library                                {#mainpage}
===================

@tableofcontents

What is AUDIO-RIVER:                               {#audio_river_mainpage_what}
====================

AUDIO-RIVER, is a multi-platform library to manage the input and output audio flow.
It can be compared with PulseAudio or Jack, but at the difference at the 2 interfaces
it is designed to be multi-platform and is based on licence that permit to integrate it
on every program we want.


What it does:                               {#audio_river_mainpage_what_it_does}
=============

Everywhere! RIVER is cross-platform devolopped to support bases OS:
: ** Linux (over Alsa, Pulseaudio, JackD)
: ** Windows (over ASIO)
: ** MacOs (over CoreAudio)
: ** Android (Over Ewol wrapper little complicated need to be change later)
: ** IOs (over CoreAudio for ios)

AUDIO-RIVER is dependent of the STL (compatible with MacOs stl (CXX))

Architecture:
-------------

River has been designed to replace the pulseAudio basic asyncronous interface that create
more problem that it will solve. The second point is that is not enougth portable to be
embended in a proprietary software without distributing all the sources (Ios).

Start at this point we will have simple objectives :
  - Manage multiple Low level interface: @ref audio_orchestra_mainpage_what
     * for linux (Alsa, Pulse, Oss)
     * for Mac-OsX (CoreAudio)
     * for IOs (coreAudio (embended version))
     * for Windows (ASIO)
     * For Android (Java (JDK...))
  - Synchronous interface ==> no delay and reduce latency
  - Manage the thread priority (need sometimes to be more reactive)
  - manage mixing of some flow (2 inputs stereo and the user want 1 input quad)
  - AEC Acoustic Echo Cancelation (TODO : in the current implementation we have a simple sound cutter)
  - Equalizer (done with @ref audio_drain_mainpage_what)
  - Resmpling (done by the libspeexDSP)
  - Correct volume management (and configurable)
  - Fade-in and Fade-out @ref audio_drain_mainpage_what
  - Channel reorganisation @ref audio_drain_mainpage_what
  - A correct feedback interface


What languages are supported?                    {#audio_river_mainpage_language}
=============================

AUDIO-RIVER is written in C++.


Are there any licensing restrictions?            {#audio_river_mainpage_license_restriction}
=====================================

AUDIO-RIVER is **FREE software** and _all sub-library are FREE and staticly linkable !!!_


License (MPL v2.0)                               {#audio_river_mainpage_license}
==================

Copyright AUDIO-RIVER Edouard DUPIN

Licensed under the Mozilla Public License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

<https://www.mozilla.org/MPL/2.0>

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Other pages                              {#audio_river_mainpage_sub_page}
===========

  - @ref audio_river_build
  - @ref audio_river_read
  - @ref audio_river_write
  - @ref audio_river_feedback
  - @ref audio_river_config_file
  - [**ewol coding style**](http://atria-soft.github.io/ewol/ewol_coding_style.html)

