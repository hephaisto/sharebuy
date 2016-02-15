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

ShareBuy::ShareBuy(const Wt::WEnvironment &env, shared_ptr<std::map<string, shared_ptr<Shop> > > shops, string databaseFile)
:Wt::WApplication(env),
dbSession(databaseFile),
shops(shops)
{
	// app metadata
	enableUpdates(true); // necessary for updates from the backend parsing web pages
	Wt::WBootstrapTheme *theme=new Wt::WBootstrapTheme();
	//theme->setVersion(3);
	setTheme(theme);

	// navbar
	Wt::WNavigationBar *nav=new Wt::WNavigationBar(this->root());
	nav->setTitle("Sharebuy");
	nav->setResponsive(true);

	Wt::WMenu *leftMenu=new Wt::WMenu();
	nav->addMenu(leftMenu);

	// my items
	Wt::WMenuItem *myItems = leftMenu->addItem("my items");
	myItems->setLink(Wt::WLink(Wt::WLink::InternalPath,"/user/items"));
	myItems->setSelectable(false);

	// my orders
	Wt::WMenuItem *myOrders = leftMenu->addItem("my orders");
	myOrders->setLink(Wt::WLink(Wt::WLink::InternalPath,"/user/orders"));
	myOrders->setSelectable(false);

	// shops
	Wt::WPopupMenu *shopMenu=new Wt::WPopupMenu();
	BOOST_FOREACH(auto tuple,*shops)
	{
		shopMenu->addItem(tuple.first)->setLink(Wt::WLink(Wt::WLink::InternalPath,"/shop/"+tuple.first));
	}
	/*Wt::WMenuItem *shopItem=new Wt::WMenuItem("Shops");
	shopItem->setMenu(shopMenu);*/
	leftMenu->addMenu("Shops",shopMenu);

	// main window
	Wt::WContainerWidget *main = new Wt::WContainerWidget(this->root());
	Wt::WBorderLayout *layout = new Wt::WBorderLayout();
	main->setLayout(layout);

	content = new Wt::WContainerWidget();
	layout->addWidget(content, Wt::WBorderLayout::Center);
	layout->addWidget(getAuthWidget(), Wt::WBorderLayout::East);

	// session state
	dbSession.login().changed().connect(this,&ShareBuy::onInternalPathChange);
	internalPathChanged().connect(this,&ShareBuy::onInternalPathChange);
	onInternalPathChange();
}

void ShareBuy::showUserItems()
{
	InputItemWidget *input = new InputItemWidget(shops,content);
	string userId = dbSession.login().user().id();
	//dbo::Query<PItem> query = dbSession.find<Item>().where("user_id = ?").bind(user.id());
	//BasketListWidget *add=new BasketListWidget(shops, query, content);
	
	BasketListWidget *pending=new BasketListWidget(shops, "", userId, "", content);
	pending->setTitle("Your items");
	pending->setCanDelete(true);
	pending->setOnlyOrderStatus(false);
	pending->update();

	input->itemEntered().connect(pending, &BasketListWidget::update);

	// find orders with items that belong to userId
	dbo::Transaction transaction(dbSession);
	dbo::collection<dbo::ptr<Order> > myOrders = dbSession.query<dbo::ptr<Order> >(
		"select ordering from ordering,item where item.user_id=? and item.ordering_id=ordering.id group by ordering.id order by ordering.id desc"
		).bind(userId);

	BOOST_FOREACH(auto order, myOrders)
	{
		BasketListWidget *ordered=new BasketListWidget(shops, "", userId, boost::lexical_cast<string>(order.id()), content);
		ordered->setTitle("Ordered by "+order->user->getUsername()); // TODO get user name from order
		ordered->setOnlyOrderStatus(true);
		ordered->update();
	}
}

void ShareBuy::showUserOrders()
{
	string userId = dbSession.login().user().id();

	dbo::Transaction transaction(dbSession);
	dbo::collection<dbo::ptr<Order> > myOrders = dbSession.find<Order >().where("user_id = ?").bind(userId).orderBy("id desc");

	BOOST_FOREACH(auto order, myOrders)
	{
		BasketListWidget *list=new BasketListWidget(shops, "", "", boost::lexical_cast<string>(order.id()), content);
		list->setTitle("Order");
		list->setShowAddToCartButton(true);
		list->setCanDelete(true);
		list->update();
	}

	if(myOrders.size()==0)
		new Wt::WText("You haven't ordered any items yet!", content);
}

void ShareBuy::showShop(string shopName)
{
	if(shops->find(shopName)!=shops->end())
	{
	
		new ItemGroupCheckbox(shopName, content);

		//dbo::Query<PItem> query = dbSession.find<Item>().where("shop_name = ?").bind(shopName);
		//BasketListWidget *add=new BasketListWidget(shops, query, content);
		BasketListWidget *add=new BasketListWidget(shops, shopName, "", "", content);
		add->setTitle("currently all items for the shop from all users");
		add->setOnlyOrderStatus(false);
		add->update();
	}
	else
		new Wt::WText("Shop "+shopName+" is not configured",content);
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
		if(internalPath()=="/user/items")
			showUserItems();
		else if(internalPath()=="/user/orders")
			showUserOrders();
		else if(internalPath().substr(0,6)=="/shop/")
		{
			string shopName = internalPath().substr(6);
			showShop(shopName);
		}
		else
			setInternalPath("/user/items", true);

	}
	else
	{
		//new Wt::WText("you have to be logged in to access this page", content);
	}
}
Wt::Auth::AuthWidget* ShareBuy::getAuthWidget()
{
	Wt::Auth::AuthWidget *authWidget=new Wt::Auth::AuthWidget(Session::auth(), dbSession.users(), dbSession.login());

	authWidget->model()->addPasswordAuth(/*dynamic_cast<Wt::Auth::AbstractPasswordService*>(*/&Session::passwordAuth());
	/*authWidget->model()->addOAuth(Session::oAuth());*/
	authWidget->setRegistrationEnabled(true);

	authWidget->processEnvironment();
	return authWidget;
}
