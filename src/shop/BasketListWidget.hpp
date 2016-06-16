#include <Wt/WContainerWidget>
#include <Wt/WPanel>
#include <memory>
#include <map>
using std::shared_ptr;

#include "Shop.hpp"
#include "Item.hpp"

class BasketListWidget : public Wt::WPanel
{
public:
	BasketListWidget(ShopList shops, dbo::Query<PItem> query, bool showRemoveButtons, Wt::WContainerWidget *parent);
	void setCanDelete(bool canDelete);
	void setShowAddToCartButton(bool showAddToCartButton);
	virtual void update() = 0;
protected:
	dbo::Query<PItem> query;
	ShopList shops;
	bool showRemoveButtons;

	std::map<dbo::ptr<User>, double> userSums;
	Wt::WContainerWidget *root;
};

/// Overview for the ordering user
// Additional widgets: add-to-cart, per-user-total, order/receive-date-setter
// Additional options: remove-buttons
class OrderOverviewForOrderer : public BasketListWidget
{
public:
	OrderOverviewForOrderer(ShopList shops, POrder order, Wt::WContainerWidget *parent);
	virtual void update();
};

/// Order status for the wishing user
// Additional widgets: total, order/receive-date-display
class OrderOverviewForWisher : public BasketListWidget
{
public:
	OrderOverviewForWisher(ShopList shops, POrder order, PUser user, Wt::WContainerWidget *parent);
	virtual void update();
};

/// Wishlist overview for ordering user
// Additional widgets: per-user-checkbox (in other widget), order-button
class WishlistForOrderer : public BasketListWidget
{
public:
	WishlistForOrderer(ShopList shops, string shopname, Wt::WContainerWidget *parent);
	virtual void update();
};

/// Wishlist overview in home panel
// Additional options: remove-buttons
class WishlistForWisher : public BasketListWidget
{
public:
	WishlistForWisher(ShopList shops, PUser user, Wt::WContainerWidget *parent);
	virtual void update();
};
