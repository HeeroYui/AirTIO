/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license APACHE v2.0 (see license file)
 */
#pragma once

#include <ewol/widget/Windows.h>
#include <ewol/widget/Composer.h>

namespace appl {
	class Windows : public ewol::widget::Windows {
		private:
			std::shared_ptr<ewol::widget::Composer> m_composer;
		protected:
			Windows();
			void init();
		public:
			DECLARE_FACTORY(Windows);
		public: // callback functions
			void onCallbackRecord();
			void onCallbackGenerate();
	};
}

