#!/usr/bin/python
import lutinModule as module
import lutinTools as tools
import lutinDebug as debug

def get_desc():
	return "airtio_test : Multi-nodal audio interface test"


def create(target):
	myModule = module.Module(__file__, 'airtio_test', 'BINARY')
	
	myModule.add_src_file([
		'test/main.cpp',
		'test/debug.cpp'
		])
	
	myModule.copy_folder('data/*')
	myModule.add_module_depend(['airtio', 'gtest', 'etk'])
	
	# add the currrent module at the 
	return myModule









