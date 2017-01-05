/** @file
 * @author Edouard DUPIN 
 * @copyright 2015, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */
#pragma once

#include <ewol/widget/Windows.hpp>
#include <ewol/widget/Composer.hpp>

namespace appl {
	class Windows : public ewol::widget::Windows {
		private:
			ememory::SharedPtr<ewol::widget::Composer> m_composer;
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

