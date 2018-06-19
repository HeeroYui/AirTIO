/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <ewol/ewol.hpp>
#include <appl/debug.hpp>
#include <appl/Windows.hpp>
#include <ewol/widget/Label.hpp>
#include <ewol/widget/Button.hpp>
#include <audio/river/widget/TemporalViewer.hpp>
#include <etk/tool.hpp>

appl::Windows::Windows() :
  m_composer(null) {
	addObjectType("appl::Windows");
	propertyTitle.setDirectCheck("River IO viewer");
}

void appl::Windows::init() {
	ewol::widget::Windows::init();
	etk::String composition = etk::String("");
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
	
	m_composer = ewol::widget::Composer::create();
	if (m_composer == null) {
		APPL_CRITICAL(" An error occured ... in the windows creatrion ...");
		return;
	}
	m_composer->loadFromString(composition);
	setSubWidget(m_composer);
	subBind(ewol::widget::Button, "bt-record", signalPressed, sharedFromThis(), &appl::Windows::onCallbackRecord);
	subBind(ewol::widget::Button, "bt-generate", signalPressed, sharedFromThis(), &appl::Windows::onCallbackGenerate);
}

void appl::Windows::onCallbackRecord() {
	ememory::SharedPtr<audio::river::widget::TemporalViewer> tmpDisp = ememory::dynamicPointerCast<audio::river::widget::TemporalViewer>(getSubObjectNamed("displayer"));
	if (tmpDisp != null) {
		tmpDisp->recordToggle();
	}
}

void appl::Windows::onCallbackGenerate() {
	ememory::SharedPtr<audio::river::widget::TemporalViewer> tmpDisp = ememory::dynamicPointerCast<audio::river::widget::TemporalViewer>(getSubObjectNamed("displayer"));
	if (tmpDisp != null) {
		tmpDisp->generateToggle();
	}
}

