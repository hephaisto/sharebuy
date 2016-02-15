#include <Wt/WContainerWidget>
#include <Wt/WPushButton>

#include "Shop.hpp"

class InputItemWidget : public Wt::WContainerWidget
{
public:
	InputItemWidget(ShopList shops, Wt::WContainerWidget *parent=NULL);
	Wt::Signal<void>& itemEntered();
private:
	Wt::WLineEdit *edit;
	Wt::WLineEdit *countEdit;
	void changed();
	ShopList shops;
	Wt::Signal<void> _itemEntered;
	Wt::WPushButton *btn;
};
