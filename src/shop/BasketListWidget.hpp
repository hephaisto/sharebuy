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
	BasketListWidget(ShopList shops, string shopName, string userId, string orderId, Wt::WContainerWidget *parent);
	//BasketListWidget(ShopList shops, dbo::Query<PItem> query, Wt::WContainerWidget *parent);
	void setCanDelete(bool canDelete);
	void setOnlyOrderStatus(bool orderStatus);
	void setShowAddToCartButton(bool showAddToCartButton);
	void update();
private:
	shared_ptr<std::map<string, shared_ptr<Shop> > > shops;
	string shopName;
	string userId;
	string orderId;
	//dbo::Query<PItem> query;
	bool canDelete;
	bool orderStatus;
	bool showOnlyOrderStatus;
	bool showAddToCartButton;
};
