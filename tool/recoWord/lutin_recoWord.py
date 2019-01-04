#!/usr/bin/python
import lutin.debug as debug
import lutin.tools as tools
import os


def get_type():
	return "BINARY"

def get_desc():
	return "Simpleaudio IO viewer and test ..."

def get_licence():
	return "MPL-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def configure(target, my_module):
	my_module.add_extra_flags()
	my_module.add_src_file([
		'appl/debug.cpp',
		'appl/main.cpp',
		'appl/Windows.cpp',
		'appl/widget/DataViewer.cpp',
		])
	my_module.add_depend(['ewol', 'audio-river'])
	my_module.add_path(".")
	# set the package properties :
	my_module.set_pkg("SECTION", ["Development"])
	my_module.set_pkg("PRIORITY", "optional")
	
	my_module.add_pkg("RIGHT", "RECORD_AUDIO")
	return True

