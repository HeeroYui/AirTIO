#!/usr/bin/python
import lutinModule as module
import lutinTools as tools
import lutinDebug as debug

def get_desc():
	return "airtio : Multi-nodal audio interface"


def create(target):
	myModule = module.Module(__file__, 'airtio', 'LIBRARY')
	
	myModule.add_src_file([
		'airtio/debug.cpp',
		'airtio/Manager.cpp',
		'airtio/Interface.cpp',
		'airtio/io/Node.cpp',
		'airtio/io/Manager.cpp'
		])
	
	myModule.add_module_depend(['airtaudio', 'airtalgo'])
	myModule.add_export_path(tools.get_current_path(__file__))
	
	# add the currrent module at the 
	return myModule








