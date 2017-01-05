#!/usr/bin/python
import lutin.debug as debug
import lutin.tools as tools
import os


def get_type():
	return "LIBRARY"

def get_desc():
	return "audio specific widget"

def get_licence():
	return "MPL-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def get_version():
	return [0,0,0]

def configure(target, my_module):
	my_module.add_src_file([
	    'audio/river/widget/TemporalViewer.cpp',
	    'audio/river/widget/debug.cpp'
	    ])
	my_module.add_header_file([
	    'audio/river/widget/TemporalViewer.hpp'
	    ])
	my_module.add_depend([
	    'ewol',
	    'audio-river'
	    ])
	my_module.add_path(".")
	return True


