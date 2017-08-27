/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */


#include <etk/types.hpp>
#include <ewol/ewol.hpp>
#include <gale/context/commandLine.hpp>

#include <appl/debug.hpp>
#include <appl/Windows.hpp>
#include <ewol/object/Object.hpp>
#include <ewol/widget/Manager.hpp>
#include <ewol/context/Context.hpp>
#include <audio/river/widget/TemporalViewer.hpp>

static const etk::String configurationRiver =
	"{\n"
	"	microphone:{\n"
	"		io:'input',\n"
	"		map-on:{\n"
	"			interface:'auto',\n"
	"			name:'default',\n"
	"		},\n"
	"		frequency:0,\n"
	"		channel-map:['front-left', 'front-right'],\n"
	"		type:'auto',\n"
	"		nb-chunk:1024\n"
	"	}\n"
	"}\n";


class MainApplication : public ewol::context::Application {
	public:
		bool init(ewol::Context& _context, size_t _initId) {
			APPL_INFO("==> Init APPL (START) [" << ewol::getBoardType() << "] (" << ewol::getCompilationMode() << ")");
			
			audio::river::initString(configurationRiver);
			
			_context.setSize(vec2(800, 600));
			
			// select internal data for font ...
			_context.getFontDefault().setUseExternal(true);
			_context.getFontDefault().setSize(19);
			
			audio::river::widget::TemporalViewer::createManagerWidget(_context.getWidgetManager());
			
			ememory::SharedPtr<ewol::widget::Windows> basicWindows = appl::Windows::create();
			// create the specific windows
			_context.setWindows(basicWindows);
			APPL_INFO("==> Init APPL (END)");
			return true;
		}
		void unInit(ewol::Context& _context) {
			APPL_INFO("==> Un-Init APPL (START)");
			// nothing to do...
			APPL_INFO("==> Un-Init APPL (END)");
		}
};

/**
 * @brief Main of the program (This can be set in every case, but it is not used in Andoid...).
 * @param std IO
 * @return std IO
 */
int main(int _argc, const char *_argv[]) {
	// second possibility
	return ewol::run(new MainApplication(), _argc, _argv);
}