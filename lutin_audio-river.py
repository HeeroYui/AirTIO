#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools


def get_type():
	return "LIBRARY"

def get_desc():
	return "Multi-nodal audio interface"

def get_licence():
	return "APACHE-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def get_version():
	return [0,3,"dev"]

def create(target, module_name):
	my_module = module.Module(__file__, module_name, get_type())
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
		'audio/river/river.h',
		'audio/river/Manager.h',
		'audio/river/Interface.h',
		'audio/river/io/Group.h',
		'audio/river/io/Node.h',
		'audio/river/io/Manager.h'
		])
	my_module.add_optionnal_module_depend('audio-orchestra', ["c++", "-DAUDIO_RIVER_BUILD_ORCHESTRA"])
	my_module.add_optionnal_module_depend('portaudio', ["c++", "-DAUDIO_RIVER_BUILD_PORTAUDIO"])
	my_module.add_module_depend(['audio', 'audio-drain', 'ejson'])
	my_module.add_path(tools.get_current_path(__file__))
	return my_module









