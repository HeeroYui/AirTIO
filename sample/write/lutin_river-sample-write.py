#!/usr/bin/python
import lutin.module as module
import lutin.tools as tools

def get_desc():
	return "Write some data"


def create(target):
	my_module = module.Module(__file__, 'river-sample-write', 'BINARY')
	my_module.add_src_file([
		'main.cpp',
		])
	my_module.add_module_depend(['audio-river', 'etk'])
	return my_module









