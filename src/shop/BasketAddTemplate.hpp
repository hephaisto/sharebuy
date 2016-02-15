#ifndef H_SHAREBUY_BASKET_ADD_TEMPLATE
#define H_SHAREBUY_BASKET_ADD_TEMPLATE

#include <map>
#include <string>
using std::string;
#include <Wt/WWidget>
#include "Item.hpp"

class BasketAddTemplate
{
public:
	virtual ~BasketAddTemplate();
	virtual Wt::WWidget* getBasketAddWidget(std::set<PItem> items) const=0;
};

#endif // H_SHAREBUY_TEMPLATE
