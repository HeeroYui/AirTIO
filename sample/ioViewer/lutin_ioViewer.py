#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools
import datetime

def get_desc():
	return "Simpleaudio IO viewer and test ..."

def create(target):
	myModule = module.Module(__file__, 'ioViewer', 'PACKAGE')
	myModule.add_extra_compile_flags()
	myModule.add_src_file([
		'appl/debug.cpp',
		'appl/main.cpp',
		'appl/Windows.cpp'])
	myModule.add_module_depend(['ewol', 'audio-river', 'audio-river-widget'])
	myModule.add_path(tools.get_current_path(__file__))
	# set the package properties :
	myModule.pkg_set("VERSION", "1.0.0")
	myModule.pkg_set("VERSION_CODE", "1")
	myModule.pkg_set("COMPAGNY_TYPE", "org")
	myModule.pkg_set("COMPAGNY_NAME", "Edouard DUPIN")
	myModule.pkg_set("MAINTAINER", ["Mr DUPIN Edouard <yui.heero@gmail.com>"])
	myModule.pkg_set("SECTION", ["Development"])
	myModule.pkg_set("PRIORITY", "optional")
	myModule.pkg_set("DESCRIPTION", "Simple wiewer")
	myModule.pkg_set("NAME", "audio-io-viewer")
	
	return myModule

