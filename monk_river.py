#!/usr/bin/python
import monkModule as module
import monkTools as tools

def get_desc():
	return "river : Multiple flow input output audio"


def create():
	# module name is 'edn' and type binary.
	myModule = module.Module(__file__, 'river', 'LIBRARY')
	# enable doculentation :
	myModule.set_website("http://heeroyui.github.io/river/")
	myModule.set_website_sources("http://github.com/heeroyui/river/")
	myModule.set_path(tools.get_current_path(__file__) + "/river/")
	myModule.set_path_general_doc(tools.get_current_path(__file__) + "/doc/")
	# add the currrent module at the 
	return myModule

