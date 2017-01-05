#!/usr/bin/python
import lutin.debug as debug
import lutin.tools as tools


def get_type():
	return "BINARY"

def get_sub_type():
	return "SAMPLE"

def get_desc():
	return "Read some data"

def get_licence():
	return "MPL-2"

def get_compagny_type():
	return "com"

def get_compagny_name():
	return "atria-soft"

def get_maintainer():
	return ["Mr DUPIN Edouard <yui.heero@gmail.com>"]

def configure(target, my_module):
	my_module.add_src_file([
	    'read.cpp',
	    ])
	my_module.add_depend([
	    'audio-river',
	    'etk'
	    ])
	return True









