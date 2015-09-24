#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools

def get_desc():
	return "Multi-nodal audio interface"


def create(target):
	my_module = module.Module(__file__, 'audio-river', 'LIBRARY')
	
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
	
	# add the currrent module at the 
	return my_module









