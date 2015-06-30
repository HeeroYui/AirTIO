#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools
import lutin.debug as debug
import os

def get_desc():
	return "audio specific widget"

def get_license():
	return "APACHE v2.0"

def create(target):
	myModule = module.Module(__file__, 'audio-river-widget', 'LIBRARY')
	myModule.add_src_file([
		'audio/river/widget/TemporalViewer.cpp',
		'audio/river/widget/debug.cpp'
		])
	myModule.add_module_depend(['ewol', 'audio-river'])
	myModule.add_export_path(tools.get_current_path(__file__))
	return myModule


