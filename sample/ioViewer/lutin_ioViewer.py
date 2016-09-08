#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools
import datetime
import os


def get_type():
	return "BINARY"

def get_desc():
	return "Simpleaudio IO viewer and test ..."

def get_licence():
	return "APACHE-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def create(target, module_name):
	my_module = module.Module(__file__, module_name, get_type())
	my_module.add_extra_flags()
	my_module.add_src_file([
		'appl/debug.cpp',
		'appl/main.cpp',
		'appl/Windows.cpp'])
	my_module.add_depend(['ewol', 'audio-river', 'audio-river-widget'])
	my_module.add_path(tools.get_current_path(__file__))
	# set the package properties :
	my_module.set_pkg("VERSION", "1.0.0")
	my_module.set_pkg("VERSION_CODE", "1")
	my_module.set_pkg("COMPAGNY_TYPE", "org")
	my_module.set_pkg("COMPAGNY_NAME", "Edouard DUPIN")
	my_module.set_pkg("MAINTAINER", ["Mr DUPIN Edouard <yui.heero@gmail.com>"])
	my_module.set_pkg("SECTION", ["Development"])
	my_module.set_pkg("PRIORITY", "optional")
	my_module.set_pkg("DESCRIPTION", "Simple wiewer")
	my_module.set_pkg("NAME", "ioViewer")
	
	my_module.add_pkg("RIGHT", "RECORD_AUDIO")
	return my_module

