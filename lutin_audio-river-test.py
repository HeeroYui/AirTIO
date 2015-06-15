#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools

def get_desc():
	return "Multi-nodal audio interface test"


def create(target):
	myModule = module.Module(__file__, 'audio-river-test', 'BINARY')
	myModule.add_src_file([
		'test/main.cpp',
		'test/debug.cpp'
		])
	myModule.add_module_depend(['audio-river', 'gtest', 'etk'])
	return myModule


