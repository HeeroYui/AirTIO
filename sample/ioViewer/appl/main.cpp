/**
 * @author Edouard DUPIN
 * 
 * @copyright 2010, Edouard DUPIN, all right reserved
 * 
 * @license BSD 3 clauses (see license file)
 */


#include <etk/types.h>
#include <ewol/ewol.h>
#include <ewol/context/commandLine.h>

#include <appl/debug.h>
#include <appl/Windows.h>
#include <ewol/object/Object.h>
#include <ewol/widget/Manager.h>
#include <ewol/context/Context.h>
#include <audio/river/widget/TemporalViewer.h>

static const std::string configurationRiver =
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
			
			std::shared_ptr<ewol::widget::Windows> basicWindows = appl::Windows::create();
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