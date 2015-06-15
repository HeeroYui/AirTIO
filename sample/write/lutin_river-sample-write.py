#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools

def get_desc():
	return "Write some data"


def create(target):
	myModule = module.Module(__file__, 'river-sample-write', 'BINARY')
	myModule.add_src_file([
		'main.cpp',
		])
	myModule.add_module_depend(['audio-river', 'etk'])
	return myModule









