/**
 * @author Edouard DUPIN
 * 
 * @copyright 2010, Edouard DUPIN, all right reserved
 * 
 * @license BSD 3 clauses (see license file)
 */

#include <ewol/ewol.h>
#include <appl/debug.h>
#include <appl/Windows.h>
#include <ewol/widget/Label.h>
#include <ewol/widget/Button.h>
#include <audio/river/widget/TemporalViewer.h>
#include <etk/tool.h>

#undef __class__
#define __class__ "Windows"

static const char* const g_eventChangeValues = "appl-change-value";
static const char* const g_eventAutoMode = "appl-change-auto";

appl::Windows::Windows() :
  m_composer(NULL) {
	addObjectType("appl::Windows");
}

void appl::Windows::init() {
	ewol::widget::Windows::init();
	setTitle("example 001_HelloWord");
	std::string composition = std::string("");
	composition += "<sizer mode='vert'>\n";
	composition += "	<sizer mode='hori' lock='true' min-size='10,10%'>\n";
	composition += "		<button name='bt-record' expend='true' fill='true'>\n";
	composition += "			<label>\n";
	composition += "				Start/Stop record\n";
	composition += "			</label>\n";
	composition += "		</button>\n";
	composition += "		<button name='bt-generate' expend='true' fill='true'>\n";
	composition += "			<label>\n";
	composition += "				Start/Stop Generate\n";
	composition += "			</label>\n";
	composition += "		</button>\n";
	composition += "	</sizer>\n";
	composition += "	<TemporalViewer name='displayer' expand='true' fill='true'/>\n";
	composition += "</sizer>\n";
	
	m_composer = ewol::widget::Composer::create(ewol::widget::Composer::String, composition);
	if (m_composer == NULL) {
		APPL_CRITICAL(" An error occured ... in the windows creatrion ...");
		return;
	}
	setSubWidget(m_composer);
	subBind(ewol::widget::Button, "bt-record", signalPressed, shared_from_this(), &appl::Windows::onCallbackRecord);
	subBind(ewol::widget::Button, "bt-generate", signalPressed, shared_from_this(), &appl::Windows::onCallbackGenerate);
}

void appl::Windows::onCallbackRecord() {
	std::shared_ptr<audio::river::widget::TemporalViewer> tmpDisp = std::dynamic_pointer_cast<audio::river::widget::TemporalViewer>(getSubObjectNamed("displayer"));
	if (tmpDisp != NULL) {
		tmpDisp->recordToggle();
	}
}

void appl::Windows::onCallbackGenerate() {
	std::shared_ptr<audio::river::widget::TemporalViewer> tmpDisp = std::dynamic_pointer_cast<audio::river::widget::TemporalViewer>(getSubObjectNamed("displayer"));
	if (tmpDisp != NULL) {
		tmpDisp->generateToggle();
	}
}

