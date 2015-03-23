=?=River extract and build examples an example=?=

All developpement software will start by getting the dependency and the sources.

=== Linux dependency packages ===
[code style=shell]
	sudo apt-get install g++ zlib1g-dev libasound2-dev
	# if you want to compile with clang :
	sudo apt-get install clang
[/code]


=== Download instructions ===

Download the software : This is the simple way You really need only a part of the ewol framework
[code style=shell]
	# create a working directory path
	mkdir your_workspace_path
	cd your_workspace_path
	# clone ewol and all sub-library
	git clone git://github.com/HeeroYui/ewol.git
	cd ewol
	git submodule init
	git submodule update
	cd ..
[/code]

If you prefer creating with the packege you needed :
[code style=shell]
	mkdir -p your_workspace_path
	cd your_workspace_path
	# download all you needs
	git clone git://github.com/HeeroYui/lutin.git
	git clone git://github.com/HeeroYui/etk.git
	git clone git://github.com/HeeroYui/audio.git
	git clone git://github.com/HeeroYui/ejson.git
	git clone git://github.com/HeeroYui/airtaudio.git
	git clone git://github.com/HeeroYui/drain.git
	git clone git://github.com/HeeroYui/river.git
[/code]

[note]
The full build tool documentation is availlable here : [[http://heeroyui.github.io/lutin/ | lutin]]
[/note]

=== Common build instructions ===

Build the basic examples & test:
[code style=shell]
	./ewol/build/lutin.py -mdebug river_sample_read
[/code]

To run an application you will find it directly on the out 'staging' tree :
[code style=shell]
	./out/Linux/debug/staging/clang/river_sample_read/usr/bin/river_sample_read -l4
[/code]

