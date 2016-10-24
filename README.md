audio-river
=====

`audio-river` is a High level hardware audio interface that connect to orchestra (multi-platform backend)

Release (master)
----------------

[![Build Status](https://travis-ci.org/musicdsp/audio-river.svg?branch=master)](https://travis-ci.org/musicdsp/audio-river)
[![Coverage Status](http://musicdsp.com/ci/coverage/musicdsp/audio-river.svg?branch=master)](http://musicdsp.com/ci/musicdsp/audio-river)
[![Test Status](http://musicdsp.com/ci/test/musicdsp/audio-river.svg?branch=master)](http://musicdsp.com/ci/musicdsp/audio-river)
[![Warning Status](http://musicdsp.com/ci/warning/musicdsp/audio-river.svg?branch=master)](http://musicdsp.com/ci/musicdsp/audio-river)

[![Build Status](http://musicdsp.com/ci/build/musicdsp/audio-river.svg?branch=master&tag=Linux)](http://musicdsp.com/ci/musicdsp/audio-river)
[![Build Status](http://musicdsp.com/ci/build/musicdsp/audio-river.svg?branch=master&tag=MacOs)](http://musicdsp.com/ci/musicdsp/audio-river)
[![Build Status](http://musicdsp.com/ci/build/musicdsp/audio-river.svg?branch=master&tag=Mingw)](http://musicdsp.com/ci/musicdsp/audio-river)

[![Build Status](http://musicdsp.com/ci/build/musicdsp/audio-river.svg?branch=master&tag=Android)](http://musicdsp.com/ci/musicdsp/audio-river)
[![Build Status](http://musicdsp.com/ci/build/musicdsp/audio-river.svg?branch=master&tag=IOs)](http://musicdsp.com/ci/musicdsp/audio-river)

Developement (dev)
------------------

[![Build Status](https://travis-ci.org/musicdsp/audio-river.svg?branch=dev)](https://travis-ci.org/musicdsp/audio-river)
[![Coverage Status](http://musicdsp.com/ci/coverage/musicdsp/audio-river.svg?branch=dev)](http://musicdsp.com/ci/musicdsp/audio-river)
[![Test Status](http://musicdsp.com/ci/test/musicdsp/audio-river.svg?branch=dev)](http://musicdsp.com/ci/musicdsp/audio-river)
[![Warning Status](http://musicdsp.com/ci/warning/musicdsp/audio-river.svg?branch=dev)](http://musicdsp.com/ci/musicdsp/audio-river)

[![Build Status](http://musicdsp.com/ci/build/musicdsp/audio-river.svg?branch=dev&tag=Linux)](http://musicdsp.com/ci/musicdsp/audio-river)
[![Build Status](http://musicdsp.com/ci/build/musicdsp/audio-river.svg?branch=dev&tag=MacOs)](http://musicdsp.com/ci/musicdsp/audio-river)
[![Build Status](http://musicdsp.com/ci/build/musicdsp/audio-river.svg?branch=dev&tag=Mingw)](http://musicdsp.com/ci/musicdsp/audio-river)

[![Build Status](http://musicdsp.com/ci/build/musicdsp/audio-river.svg?branch=dev&tag=Android)](http://musicdsp.com/ci/musicdsp/audio-river)
[![Build Status](http://musicdsp.com/ci/build/musicdsp/audio-river.svg?branch=dev&tag=IOs)](http://musicdsp.com/ci/musicdsp/audio-river)


Instructions
============

download Build system:
----------------------

	sudo pip install lutin
	sudo pip install pillow

download the software:
----------------------

	mkdir WORKING_DIRECTORY
	cd WORKING_DIRECTORY
	git clone https://github.com/atria-soft/etk.git
	git clone https://github.com/atria-soft/elog.git
	git clone https://github.com/atria-soft/ememory.git
	git clone https://github.com/atria-soft/ethread.git
	git clone https://github.com/atria-soft/ejson.git
	git clone https://github.com/musicdsp/audio.git
	git clone https://github.com/musicdsp/audio-algo-drain.git
	git clone https://github.com/musicdsp/audio-algo-river.git
	git clone https://github.com/musicdsp/audio-algo-speex.git
	git clone https://github.com/musicdsp/audio-drain.git
	git clone https://github.com/musicdsp/audio-orchestra.git
	git clone https://github.com/musicdsp/audio-river.git
	git clone https://github.com/generic-library/gtest-lutin.git --recursive
	git clone https://github.com/generic-library/z-lutin.git --recursive
	git clone https://github.com/generic-library/speex-dsp-lutin.git --recursive

Compile software:
-----------------

	cd WORKING_DIRECTORY
	lutin -C -P audio-river-test?build?run

License (APACHE v2.0)
=====================
Copyright audio-river Edouard DUPIN

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

