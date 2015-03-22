#!/usr/bin/python
import lutinModule as module
import lutinTools as tools
import lutinDebug as debug

def get_desc():
	return "river_sample_read : Read some data"


def create(target):
	myModule = module.Module(__file__, 'river_sample_read', 'BINARY')
	
	myModule.add_src_file([
		'main.cpp',
		])
	myModule.add_module_depend(['river', 'etk'])
	
	return myModule









