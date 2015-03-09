#!/usr/bin/python
import lutinModule as module
import lutinTools as tools
import lutinDebug as debug

def get_desc():
	return "river_test : Multi-nodal audio interface test"


def create(target):
	myModule = module.Module(__file__, 'river_test', 'BINARY')
	
	myModule.add_src_file([
		'test/main.cpp',
		'test/debug.cpp'
		])
	
	if target.name=="Windows":
		myModule.copy_file('data/hardwareWindows.json', 'hardware.json')
	elif target.name=="Linux":
		#myModule.copy_file('data/hardwareLinux.json', 'hardware.json')
		myModule.copy_file('data/hardwareNao.json', 'hardware.json')
	elif target.name=="MacOs":
		myModule.copy_file('data/hardwareMacOs.json', 'hardware.json')
	elif target.name=="IOs":
		myModule.copy_file('data/hardwareIOs.json', 'hardware.json')
	elif target.name=="Android":
		myModule.copy_file('data/hardwareAndroid.json', 'hardware.json')
	else:
		debug.warning("unknow target for AIRTAudio : " + target.name);
	
	myModule.copy_file('data/virtual.json', 'virtual.json')
	myModule.add_module_depend(['river', 'gtest', 'etk'])
	
	# add the currrent module at the 
	return myModule









