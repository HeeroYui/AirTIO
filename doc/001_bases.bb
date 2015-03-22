=?=RIVER: Bases =?=
__________________________________________________
[right][tutorial[000_Build | Next: Tutorals]][/right]

=== Overview:===

===User requires:===
To use ewol you need to know only C++ language. It could be usefull to know:
:** [b]Python[/b] for all build tool.
:** [b]git[/b] for all version management
:** [b]Audio[/b] Basic knowlege of audio streaming af data organisation.

=== Architecture:===
River has been designed to replace the pulseAudio basic asyncronous interface that create 
more problem that it will solve. The second point is that is not enougth portable to be 
embended in a proprietary software without distributing all the sources (Ios).

Start at this point we will have simple objectives :
:** manage multiple Low level interface: (done by the [lib[airtaudio | AirTAudio]] interface):
::** for linux
:::** Alsa
:::** Pulse
:::** Oss
::** for Mac-OsX
:::** CoreAudio
::** for IOs
:::** CoreAudio (embended version)
::** for Windows
:::** ASIO
::** For Android
:::** Java (JDK-6)
:** Synchronous interface ==> no delay and reduce latency
:** Manage the thread priority (need sometimes to be more reactive)
:** manage mixing of some flow (2 inputs stereo and the user want 1 input quad)
:** AEC Acoustic Echo Cancelation (TODO : in the current implementation we have a simple sound cutter)
:** Equalizer (done with [lib[drain | Drain])
:** Resmpling (done by the libspeexDSP)
:** Correct volume management (and configurable)
:** Fade-in and Fade-out (done with [lib[drain | Drain])
:** Channel reorganisation (done with [lib[drain | Drain])
:** A correct feedback interface
