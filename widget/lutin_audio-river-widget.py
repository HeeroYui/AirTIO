#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools
import lutin.debug as debug
import os


def get_type():
	return "LIBRARY"

def get_desc():
	return "audio specific widget"

def get_licence():
	return "APACHE-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def get_version():
	return [0,0,0]

def create(target, module_name):
	my_module = module.Module(__file__, module_name, get_type())
	my_module.add_src_file([
		'audio/river/widget/TemporalViewer.cpp',
		'audio/river/widget/debug.cpp'
		])
	my_module.add_header_file([
		'audio/river/widget/TemporalViewer.hpp'
		])
	my_module.add_depend(['ewol', 'audio-river'])
	my_module.add_path(tools.get_current_path(__file__))
	return my_module


