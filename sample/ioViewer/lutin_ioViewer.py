#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools
import datetime

def get_desc():
	return "Simpleaudio IO viewer and test ..."

def create(target):
	my_module = module.Module(__file__, 'ioViewer', 'PACKAGE')
	my_module.add_extra_compile_flags()
	my_module.add_src_file([
		'appl/debug.cpp',
		'appl/main.cpp',
		'appl/Windows.cpp'])
	my_module.add_module_depend(['ewol', 'audio-river', 'audio-river-widget'])
	my_module.add_path(tools.get_current_path(__file__))
	# set the package properties :
	my_module.pkg_set("VERSION", "1.0.0")
	my_module.pkg_set("VERSION_CODE", "1")
	my_module.pkg_set("COMPAGNY_TYPE", "org")
	my_module.pkg_set("COMPAGNY_NAME", "Edouard DUPIN")
	my_module.pkg_set("MAINTAINER", ["Mr DUPIN Edouard <yui.heero@gmail.com>"])
	my_module.pkg_set("SECTION", ["Development"])
	my_module.pkg_set("PRIORITY", "optional")
	my_module.pkg_set("DESCRIPTION", "Simple wiewer")
	my_module.pkg_set("NAME", "ioViewer")
	
	my_module.pkg_add("RIGHT", "RECORD_AUDIO")
	return my_module

