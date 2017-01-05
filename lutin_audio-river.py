#!/usr/bin/python
import lutin.debug as debug
import lutin.tools as tools


def get_type():
	return "LIBRARY"

def get_desc():
	return "Multi-nodal audio interface"

def get_licence():
	return "MPL-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return "authors.txt"

def get_version():
	return "version.txt"

def configure(target, my_module):
	my_module.add_src_file([
	    'audio/river/debug.cpp',
	    'audio/river/river.cpp',
	    'audio/river/Manager.cpp',
	    'audio/river/Interface.cpp',
	    'audio/river/io/Group.cpp',
	    'audio/river/io/Node.cpp',
	    'audio/river/io/NodeOrchestra.cpp',
	    'audio/river/io/NodePortAudio.cpp',
	    'audio/river/io/NodeAEC.cpp',
	    'audio/river/io/NodeMuxer.cpp',
	    'audio/river/io/Manager.cpp'
	    ])
	my_module.add_header_file([
	    'audio/river/river.hpp',
	    'audio/river/Manager.hpp',
	    'audio/river/Interface.hpp',
	    'audio/river/io/Group.hpp',
	    'audio/river/io/Node.hpp',
	    'audio/river/io/Manager.hpp'
	    ])
	my_module.add_optionnal_depend('audio-orchestra', ["c++", "-DAUDIO_RIVER_BUILD_ORCHESTRA"])
	my_module.add_optionnal_depend('portaudio', ["c++", "-DAUDIO_RIVER_BUILD_PORTAUDIO"])
	my_module.add_depend([
	    'audio',
	    'audio-drain',
	    'ejson'
	    ])
	my_module.add_path(".")
	return True









