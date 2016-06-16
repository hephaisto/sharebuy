#include <Wt/WApplication>
#include <Wt/Auth/AuthWidget>
#include "shop/Shop.hpp"
#include "user/Session.hpp"

static const boost::format priceFmt("%|1.2f|");

class ShareBuy : public Wt::WApplication
{
public:
	void onInternalPathChange();
	ShareBuy(const Wt::WEnvironment &env, shared_ptr<std::map<string, shared_ptr<Shop> > > shops, string databaseFile);
	Session dbSession;
	Wt::Auth::AuthWidget *getAuthWidget();

private:
	void showUserItems();
	void showUserOrders();
	void showUserSettings();
	void showShop(string shopName);
	void showUserProfile(string userId);
	void showAbout();
	shared_ptr<std::map<string, shared_ptr<Shop> > > shops;
	Wt::WContainerWidget *content;
	void createAuthWidget();
	Wt::Auth::AuthWidget *authWidget;
	//Wt::WStackedWidget *contentsStack;
};
