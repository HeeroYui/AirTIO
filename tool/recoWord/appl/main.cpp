/** @file
 * @author Edouard DUPIN 
 * @copyright 2019, Edouard DUPIN, all right reserved
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
#include <appl/widget/DataViewer.hpp>
#include <etk/theme/theme.hpp>

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
		virtual void onCreate(ewol::Context& _context) override {
			APPL_INFO(" == > CREATE ... (START) [" << gale::getBoardType() << "] (" << gale::getCompilationMode() << ") (BEGIN)");
			for( int32_t iii=0 ; iii<_context.getCmd().size(); iii++) {
				etk::String tmpppp = _context.getCmd().get(iii);
				if (    tmpppp == "-h"
				     || tmpppp == "--help") {
					//APPL_PRINT("  --ctags=xxx c-flags-file-name" );
					exit(0);
				} 
			}
			etk::theme::setName("COLOR", "color/white/");
			
			_context.setSize(vec2(800, 600));
			
			_context.setTitle("edn");
			
			// select internal data for font ...
			_context.getFontDefault().setUseExternal(true);
			#ifdef __TARGET_OS__Android
				_context.getFontDefault().set("FreeSerif", 19);
			#else
				_context.getFontDefault().set("FreeSerif;DejaVuSansMono",14);
			#endif
			
			// set the application icon ...
			//_context.setIcon("DATA:///icon.png");
			APPL_INFO("==> CREATE ... (END)");
		}
		
		void onStart(ewol::Context& _context) override {
			APPL_INFO("==> START ... (BEGIN)");
			// init internal global value
			audio::river::initString(configurationRiver);
			
			_context.setSize(vec2(800, 600));
			
			// select internal data for font ...
			_context.getFontDefault().setUseExternal(true);
			_context.getFontDefault().setSize(19);
			
			appl::widget::DataViewer::createManagerWidget(_context.getWidgetManager());
			
			ememory::SharedPtr<ewol::widget::Windows> basicWindows = appl::Windows::create();
			if (basicWindows == null) {
				APPL_ERROR("Can not allocate the basic windows");
				_context.exit(-1);
				return;
			}
			// create the specific windows
			_context.setWindows(basicWindows);
			
			// add files
			APPL_INFO("show list of files : ");
			/*
			for( int32_t iii=0 ; iii<_context.getCmd().size(); iii++) {
				etk::String tmpppp = _context.getCmd().get(iii);
				if (tmpppp.startWith("--ctags=") == true) {
					etk::String name = tmpppp.extract(8);
					APPL_INFO("Load ctag file : \"" << name << "\"" );
					appl::setCtagsFileName(name);
				} else if (    tmpppp == "-h"
				            || tmpppp == "--help") {
					// nothing to do ...
				} else {
					etk::Path file(tmpppp);
					if (etk::path::isFile(file) == true) {
						APPL_INFO("need load file : \"" << file << "\"" );
						m_bufferManager->open(file);
					} else if (etk::path::isDirectory(file) == true) {
						etk::Vector<etk::Path> listOfFiles = etk::path::list(file, etk::path::LIST_FILE);
						for (auto &it: listOfFiles) {
							if (etk::path::isFile(it) == true) {
								APPL_INFO("need load file : \"" << it << "\"" );
								m_bufferManager->open(it);
							}
						}
					}
				}
			}
			*/
			APPL_INFO("==> START ... (END)");
			return;
		}
		void onStop(ewol::Context& _context) override {
			APPL_INFO("==> STOP ... (START)");
			APPL_INFO("==> STOP ... (END)");
		}
		void onKillDemand(ewol::Context& _context) override {
			APPL_INFO("==> User demand kill ... (START)");
			_context.exit(0);
			APPL_INFO("==> User demand kill ... (END)");
		}
};

/**
 * @brief Main of the program (This can be set in every case, but it is not used in Andoid...).
 * @param std IO
 * @return std IO
 */
int main(int _argc, const char *_argv[]) {
	// second possibility
	return ewol::run(ETK_NEW(MainApplication), _argc, _argv);
}