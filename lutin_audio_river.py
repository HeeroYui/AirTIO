#!/usr/bin/python
import lutinModule as module
import lutinTools as tools
import lutinDebug as debug

def get_desc():
	return "audio_river : Multi-nodal audio interface"


def create(target):
	myModule = module.Module(__file__, 'audio_river', 'LIBRARY')
	
	myModule.add_src_file([
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
	myModule.add_optionnal_module_depend('audio_orchestra', "AUDIO_RIVER_BUILD_ORCHESTRA")
	myModule.add_optionnal_module_depend('portaudio', "AUDIO_RIVER_BUILD_PORTAUDIO")
	myModule.add_module_depend(['audio', 'audio_drain', 'ejson'])
	myModule.add_export_path(tools.get_current_path(__file__))
	
	# add the currrent module at the 
	return myModule









