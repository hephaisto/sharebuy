#include "application.hpp"

#include <boost/foreach.hpp>
#include <Wt/WBootstrapTheme>
#include <Wt/WNavigationBar>
#include <Wt/WPopupMenu>
#include <Wt/WStackedWidget>
#include <Wt/WText>
#include <Wt/Auth/AbstractPasswordService>
#include <Wt/WBorderLayout>

#include "shop/BasketListWidget.hpp"
#include "shop/InputItemWidget.hpp"
#include "shop/ItemGroupCheckbox.hpp"
#include "UserSettings.hpp"
#include "user/UserDisplay.hpp"
#include "version.hpp"

ShareBuy::ShareBuy(const Wt::WEnvironment &env, shared_ptr<std::map<string, shared_ptr<Shop> > > shops, string databaseFile)
:Wt::WApplication(env),
dbSession(databaseFile),
shops(shops),
authWidget(NULL)
{
	// app metadata
	enableUpdates(true); // necessary for updates from the backend parsing web pages
	Wt::WBootstrapTheme *theme=new Wt::WBootstrapTheme();
	theme->setVersion(Wt::WBootstrapTheme::Version::Version3);
	setTheme(theme);

	// navbar
	Wt::WNavigationBar *nav=new Wt::WNavigationBar(this->root());
	nav->setTitle("Sharebuy");
	nav->setResponsive(true);

	Wt::WMenu *leftMenu=new Wt::WMenu();
	nav->addMenu(leftMenu);

	// my items
	Wt::WMenuItem *myItems = leftMenu->addItem("My wishlist");
	myItems->setLink(Wt::WLink(Wt::WLink::InternalPath,"/user/wishlist"));
	myItems->setSelectable(false);

	// shops
	Wt::WPopupMenu *shopMenu=new Wt::WPopupMenu();
	BOOST_FOREACH(auto tuple,*shops)
	{
		shopMenu->addItem(tuple.first)->setLink(Wt::WLink(Wt::WLink::InternalPath,"/shop/"+tuple.first));
	}
	/*Wt::WMenuItem *shopItem=new Wt::WMenuItem("Shops");
	shopItem->setMenu(shopMenu);*/
	leftMenu->addMenu("All user's wishlists",shopMenu);

	// my orders
	Wt::WMenuItem *myOrders = leftMenu->addItem("Stuff I ordered");
	myOrders->setLink(Wt::WLink(Wt::WLink::InternalPath,"/user/orders"));
	myOrders->setSelectable(false);

	// RIGHT MENU
	Wt::WMenu *rightMenu = new Wt::WMenu();
	nav->addMenu(rightMenu, Wt::AlignRight);

	// settings
	Wt::WMenuItem *settings = rightMenu->addItem("Settings");
	settings->setLink(Wt::WLink(Wt::WLink::InternalPath,"/user/settings"));
	settings->setSelectable(false);

	// about
	Wt::WMenuItem *about = rightMenu->addItem("About Sharebuy");
	about->setLink(Wt::WLink(Wt::WLink::InternalPath,"/about"));
	about->setSelectable(false);

	// MAIN WINDOW
	
	// main window
	Wt::WContainerWidget *main = new Wt::WContainerWidget(this->root());
	Wt::WBorderLayout *layout = new Wt::WBorderLayout();
	main->setLayout(layout);

	// content & auth
	content = new Wt::WContainerWidget();
	layout->addWidget(content, Wt::WBorderLayout::Center);
	createAuthWidget();
	layout->addWidget(authWidget, Wt::WBorderLayout::East);

	// session state
	dbSession.login().changed().connect(this,&ShareBuy::onInternalPathChange);
	internalPathChanged().connect(this,&ShareBuy::onInternalPathChange);
	onInternalPathChange();
}

void ShareBuy::showUserSettings()
{
	PUser user = dbSession.user();
	new UserSettings(content);
}

void ShareBuy::showAbout()
{
	auto text = boost::format("This server is running ShareBuy version <b>%1%</b>. Visit <a href=\"%2%\">%2%</a> for more information.") % VERSION_STRING % "https://github.com/hephaisto/sharebuy";
	new Wt::WText(text.str(), content);
}

void ShareBuy::showUserItems()
{
	new Wt::WText("<h1>Your personal wishlist</h1>", content);
	new Wt::WText("<p>Paste the URL for the item and the number of items below.</p>", content);
	InputItemWidget *input = new InputItemWidget(shops,content);
	
	PUser user = dbSession.user();

	WishlistForWisher *p = new WishlistForWisher(shops, user, content);
	p->update();
	input->itemEntered().connect(p, &WishlistForWisher::update);

	// find orders with items that belong to userId
	dbo::Transaction transaction(dbSession);
	dbo::collection<dbo::ptr<Order> > myOrders = dbSession.query<dbo::ptr<Order> >(
		"select ordering from ordering,item where item.user_id=? and item.ordering_id=ordering.id group by ordering.id order by ordering.id desc"
		).bind(user.id());

	BOOST_FOREACH(auto order, myOrders)
	{
		OrderOverviewForWisher *ordered=new OrderOverviewForWisher(shops, order, user, content);
		ordered->update();
	}
}

void ShareBuy::showUserOrders()
{
	new Wt::WText("<h1>Items you ordered for other users</h1>", content);

	dbo::Transaction transaction(dbSession);
	dbo::collection<dbo::ptr<Order> > myOrders = dbSession.find<Order >().where("user_id = ?").bind(dbSession.user().id()).orderBy("id desc");

	BOOST_FOREACH(auto order, myOrders)
	{
		OrderOverviewForOrderer *list=new OrderOverviewForOrderer(shops, order, content);
		list->update();
	}

	if(myOrders.size()==0)
		new Wt::WText("You haven't ordered any items yet!", content);
}

void ShareBuy::showShop(string shopName)
{
	new Wt::WText("<h1>Items other users want from "+shopName+"</h1>", content);
	if(shops->find(shopName)!=shops->end())
	{
		ItemGroupCheckbox *groupCb = new ItemGroupCheckbox(shopName, content);
		groupCb->setTitle("Which users to order for?");

		WishlistForOrderer *add=new WishlistForOrderer(shops, shopName, content);
		add->update();
	}
	else
		new Wt::WText("Shop "+shopName+" is not configured",content);
}

void ShareBuy::showUserProfile(string userId)
{
	dbo::Transaction transaction(dbSession);

	PUser user = dbSession.find<User>().where("id = ?").bind(userId);

	new UserDisplay(user, dbSession.user()->isAdmin, content);
}

void ShareBuy::onInternalPathChange()
{
	content->clear();

	if(dbSession.login().loggedIn())
	{
		
		/*
		shared_ptr<Shop> shop = nullptr;
		BOOST_FOREACH(auto s, *shops)
		{
			shop = s;
		}*/

		//dbo::Query<PItem> query = dbSession.find<Item>().where("shop_name = ?").bind(shop->name);
		if(internalPath()=="/user/wishlist")
			showUserItems();
		else if(internalPath()=="/user/orders")
			showUserOrders();
		else if(internalPath()=="/user/settings")
			showUserSettings();
		else if(internalPath()=="/about")
			showAbout();
		else if(internalPath().substr(0,14)=="/user/profile/")
		{
			string userId = internalPath().substr(14);
			showUserProfile(userId);
		}
		else if(internalPath().substr(0,6)=="/shop/")
		{
			string shopName = internalPath().substr(6);
			showShop(shopName);
		}
		else
			setInternalPath("/user/wishlist", true);

	}
	else
	{
		//new Wt::WText("you have to be logged in to access this page", content);
	}
}
void ShareBuy::createAuthWidget()
{
	if(authWidget)
		delete authWidget;

	authWidget=new Wt::Auth::AuthWidget(Session::auth(), dbSession.users(), dbSession.login());

	authWidget->model()->addPasswordAuth(/*dynamic_cast<Wt::Auth::AbstractPasswordService*>(*/&Session::passwordAuth());
	/*authWidget->model()->addOAuth(Session::oAuth());*/
	authWidget->setRegistrationEnabled(true);
	authWidget->processEnvironment();
}

Wt::Auth::AuthWidget* ShareBuy::getAuthWidget()
{
	return authWidget;
}
