== [center]RIVER library[/center] ==
__________________________________________________

===What is RIVER, and how can I use it?===
RIVER is a multi-platform library to manage the input and output audio flow.
It can be compared with PulseAudio or Jack, but at the difference at the 2 interfaces
it is designed to be multi-platform and is based on licence that permit to integrate it
on every program we want.

===Where can I use it?===
Everywhere! RIVER is cross-platform devolopped to support bases OS:
: ** Linux (over Alsa, Pulseaudio, JackD)
: ** Windows (over ASIO)
: ** MacOs (over CoreAudio)
: ** Android (Over Ewol wrapper little complicated need to be change later)
: ** IOs (over CoreAudio for ios)

===What languages are supported?===
RIVER is written in C++11 with posibilities to compile it with C++03 + Boost

===Are there any licensing restrictions?===
RIVER is [b]FREE software[/b] and [i]all sub-library are FREE and staticly linkable !!![/i]

That allow you to use it for every program you want, including those developing proprietary software, without any license fees or royalties.

[note]The static support is important for some platform like IOs, and this limit the external library use at some license like :
:** BSD*
:** MIT
:** APPACHE-2
:** PNG
:** ZLIB
This exclude the classical extern library with licence:
:** L-GPL
:** GPL
[/note]

==== License (APACHE 2) ====
Copyright ewol Edouard DUPIN

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	[[http://www.apache.org/licenses/LICENSE-2.0]]

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


==== Depends library: ====
===== License: =====
:** [b][lib[etk | e-tk]][/b] : APACHE-2
:** [b][lib[airtaudio | airtaudio]][/b] : MIT/APACHE-2
:** [b][lib[ejson | e-json]][/b] : APACHE-2
:** [b][lib[drain | Drain]][/b] : APACHE-2


===== Program Using RIVER =====
:** [b][[http://play.google.com/store/apps/details?id=com.edouarddupin.worddown | worddown]][/b] : (Proprietary) Worddown is a simple word game threw [lib[ewolsa | ewol-simple-audio]].

== Main documentation: ==

[doc[001_bases | Global Documantation]]

[tutorial[000_Build | Tutorials]]

