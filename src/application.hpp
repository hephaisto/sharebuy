#include <Wt/WApplication>
#include <Wt/Auth/AuthWidget>
#include "shop/Shop.hpp"
#include "user/Session.hpp"

class ShareBuy : public Wt::WApplication
{
public:
	void onInternalPathChange();
	ShareBuy(const Wt::WEnvironment &env, shared_ptr<std::map<string, shared_ptr<Shop> > > shops, string databaseFile);
	Session dbSession;
private:
	void showUserItems();
	void showUserOrders();
	void showShop(string shopName);
	shared_ptr<std::map<string, shared_ptr<Shop> > > shops;
	Wt::WContainerWidget *content;
	Wt::Auth::AuthWidget *getAuthWidget();
	//Wt::WStackedWidget *contentsStack;
};
