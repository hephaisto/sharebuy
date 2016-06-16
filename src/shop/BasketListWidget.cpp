#include "BasketListWidget.hpp"

#include <Wt/WText>
#include <Wt/WTable>
#include <Wt/WImage>
#include <Wt/WPushButton>
#include <boost/format.hpp>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/log/trivial.hpp>
#include <boost/lexical_cast.hpp>

#include "../user/Session.hpp"
#include "../application.hpp"

BasketListWidget::BasketListWidget(ShopList shops, dbo::Query<PItem> query, bool showRemoveButtons, Wt::WContainerWidget *parent)
:Wt::WPanel(parent),
query(query),
shops(shops),
showRemoveButtons(showRemoveButtons)
{
	setCollapsible(true);
	root = new Wt::WContainerWidget();
	setCentralWidget(root);
}

void BasketListWidget::update()
{
	root->clear();
	userSums.clear();
	Session& dbSession = static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession;
	dbo::Transaction transaction(dbSession);

	PItems database_items = query; // execute query
	std::set<PItem> items(database_items.begin(),database_items.end());

	// main table
	Wt::WTable *table = new Wt::WTable(root);
	table->setHeaderCount(1);
	table->setWidth(Wt::WLength("100%"));
	table->setStyleClass("table");

	table->elementAt(0, 0)->addWidget(new Wt::WText(""));
	table->elementAt(0, 1)->addWidget(new Wt::WText("Number"));
	table->elementAt(0, 2)->addWidget(new Wt::WText("Title"));
	table->elementAt(0, 3)->addWidget(new Wt::WText("Count"));
	table->elementAt(0, 4)->addWidget(new Wt::WText("Price"));
	table->elementAt(0, 5)->addWidget(new Wt::WText("Total"));


	size_t number=0;
	BOOST_FOREACH(PItem item,items)
	{
		std::cout<<"item "<<number<<"\n";
		shared_ptr<Shop> shop = (*shops)[item->shop_name];
		string imageURL = item->formatInto(shop->imageTemplate, number);
		table->elementAt(number+1,0)->addWidget(new Wt::WImage(imageURL));
		table->elementAt(number+1,1)->addWidget(new Wt::WText(item->shop_specific_id+" / "+item->shop_specific_id_2));
		table->elementAt(number+1,2)->addWidget(new Wt::WAnchor(Wt::WLink(item->url),item->title));
		table->elementAt(number+1,3)->addWidget(new Wt::WText(boost::lexical_cast<string>(item->count)));
		table->elementAt(number+1,4)->addWidget(new Wt::WText((boost::format(priceFmt) % item->price).str()));
		table->elementAt(number+1,5)->addWidget(new Wt::WText((boost::format(priceFmt) % (item->count*item->price)).str()));
		userSums[item->user]+=item->count*item->price;
		if( showRemoveButtons )
		{
			Wt::WPushButton *btn = new Wt::WPushButton("remove");
			if( item->order)
			{
				btn->clicked().connect([this, &dbSession, item](Wt::WMouseEvent event) mutable
				{
					dbo::Transaction transaction(dbSession);
					item->order.modify()->items.erase(item);
					transaction.commit();
					update();
				});
			}
			else
			{
				btn->clicked().connect([this, &dbSession, item](Wt::WMouseEvent event) mutable
				{
					dbo::Transaction transaction(dbSession);
					item.remove();
					transaction.commit();
					update();
				});
			}
			table->elementAt(number+1,6)->addWidget(btn);
		}
		number++;
	}
}


OrderOverviewForOrderer::OrderOverviewForOrderer(ShopList shops, POrder order, Wt::WContainerWidget *parent)
//:BasketListWidget(shops, "", PUser(), orderId, parent)
:BasketListWidget(shops, static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession.find<Item>().where("ordering_id = ?").bind(order.id()).orderBy("user_id"), !order->orderTime.isValid(), parent)
{
	setTitleBar(true);

	Wt::WContainerWidget *title = new Wt::WContainerWidget(titleBarWidget());
	title->addStyleClass("accordion-toggle");

	title->addWidget(new Wt::WText("Order"));

	confirmOrder = new Wt::WContainerWidget();
	confirmOrder->setInline(true);
	title->addWidget(confirmOrder);

	confirmReceive = new Wt::WContainerWidget();
	confirmReceive->setInline(true);
	title->addWidget(confirmReceive);

	if(order->orderTime.isValid())
	{
		confirmOrder->addWidget(new Wt::WText("; ordered "+order->orderTime.timeTo(Wt::WDateTime::currentDateTime())+" ago"));
		if(order->receiveTime.isValid())
		{
			confirmReceive->addWidget(new Wt::WText("; received "+order->receiveTime.timeTo(Wt::WDateTime::currentDateTime())+" ago"));
		}
		else
		{
			Wt::WPushButton *btn = new Wt::WPushButton("received");
			btn->clicked().connect(std::bind([this, order, btn]()
			{
				Session& dbSession = static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession;
				dbo::Transaction transaction(dbSession);

				order.modify()->receiveTime = Wt::WDateTime::currentDateTime();
				delete btn;
				confirmReceive->addWidget(new Wt::WText("; received "+order->receiveTime.timeTo(Wt::WDateTime::currentDateTime())+" ago"));

				update();
			}));
			title->addWidget(btn);
		}
	}
	else
	{
		Wt::WPushButton *btn = new Wt::WPushButton("confirm");
		btn->clicked().connect(std::bind([this, order, btn]()
		{
			Session& dbSession = static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession;
			dbo::Transaction transaction(dbSession);

			order.modify()->orderTime = Wt::WDateTime::currentDateTime();
			delete btn;
			confirmOrder->addWidget(new Wt::WText("; ordered "+order->orderTime.timeTo(Wt::WDateTime::currentDateTime())+" ago"));

			showRemoveButtons = false;
			update();
		}));
		title->addWidget(btn);
	}
}

void OrderOverviewForOrderer::update()
{
	BasketListWidget::update();
	
	Session& dbSession = static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession;
	dbo::Transaction transaction(dbSession);

	PItems database_items = query; // execute query
	std::set<PItem> items(database_items.begin(),database_items.end());

	// show per-user financial overview
	//if(userId == "")
	{
		double totalSum = 0.0;
		Wt::WTable *table2 = new Wt::WTable(root);
		table2->setHeaderCount(1);
		table2->setWidth(Wt::WLength("30%"));
		table2->elementAt(0, 0)->addWidget(new Wt::WText("User"));
		table2->elementAt(0, 1)->addWidget(new Wt::WText("Total"));

		size_t row=1;
		BOOST_FOREACH(auto it, userSums)
		{
			totalSum+=it.second;
			table2->elementAt(row, 0)->addWidget(new Wt::WAnchor(Wt::WLink(Wt::WLink::Type::InternalPath, "/user/profile/"+boost::lexical_cast<string>(it.first.id())),it.first->getUsername()));
			table2->elementAt(row, 1)->addWidget(new Wt::WText((boost::format(priceFmt) % it.second).str()));
			row++;
		}
		if(userSums.size()>1)
		{
			table2->elementAt(row, 0)->addWidget(new Wt::WText("Total"));
			table2->elementAt(row, 1)->addWidget(new Wt::WText((boost::format(priceFmt) % totalSum).str()));
		}
	}


	if( !(items.empty()) )
	{
		std::cerr<<"item!\n";
		auto it = items.begin();
		string itemShopName = (*it)->shop_name;
		bool ok = true;
		for(;it!=items.end();it++)
		{
			std::cerr<<"item!\n";
			if((*it)->shop_name!=itemShopName)
			{
				ok=false;
				break;
			}
		}
		if(ok)
			root->addWidget((*shops)[itemShopName]->basketAdd->getBasketAddWidget(items));
	}


}

OrderOverviewForWisher::OrderOverviewForWisher(ShopList shops, POrder order, PUser user, Wt::WContainerWidget *parent)
:BasketListWidget(shops, static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession.find<Item>().where("ordering_id = ?").bind(order.id()).where("user_id = ?").bind(user.id()), false, parent)
{
	setTitleBar(true);

	Wt::WContainerWidget *title = new Wt::WContainerWidget(titleBarWidget());
	title->addStyleClass("accordion-toggle");

	title->addWidget(new Wt::WText("Ordered by "));
	title->addWidget(new Wt::WAnchor(Wt::WLink(Wt::WLink::Type::InternalPath, "/user/profile/"+boost::lexical_cast<string>(order->user.id())),order->user->getUsername()));
	title->addWidget(new Wt::WText("; total: "));
	totalDisplay = new Wt::WText("???");
	title->addWidget(totalDisplay);

	if(order->orderTime.isValid())
	{
		title->addWidget(new Wt::WText("; ordered "+order->orderTime.timeTo(Wt::WDateTime::currentDateTime())+" ago"));
		if(order->receiveTime.isValid())
			title->addWidget(new Wt::WText("; received "+order->receiveTime.timeTo(Wt::WDateTime::currentDateTime())+" ago"));
		else
			title->addWidget(new Wt::WText("; not received yet"));
	}
	else
		title->addWidget(new Wt::WText("; order not confirmed yet"));
}

void OrderOverviewForWisher::update()
{
	BasketListWidget::update();
	double totalSum = userSums.begin()->second; // only one entry -> use the first
	totalDisplay->setText("<b>"+(boost::format(priceFmt) % totalSum).str()+"</b>");
}




WishlistForOrderer::WishlistForOrderer(ShopList shops, string shopname, Wt::WContainerWidget *parent)
:BasketListWidget(shops, static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession.find<Item>().where("shop_name = ?").bind(shopname).where("ordering_id is null"), false, parent)
{
	setTitle("Detailed list of items");
}

void WishlistForOrderer::update()
{
	BasketListWidget::update();
}

WishlistForWisher::WishlistForWisher(ShopList shops, PUser user, Wt::WContainerWidget *parent)
:BasketListWidget(shops, static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession.find<Item>().where("user_id = ?").bind(user.id()).where("ordering_id is null"), true, parent)
{
	setTitle("Your items");
}

void WishlistForWisher::update()
{
	BasketListWidget::update();
}

/*
	dbo::Query<PItem> query = dbSession.find<Item>();
	if(user)
	{
		query.where("user_id = ?").bind(user.id());
	}
	else
		query.orderBy("user_id");
	if(orderId != "")
	{
		query.where("ordering_id = ?").bind(orderId);
	}
	if(shopName != "")
	{
		query.where("shop_name = ?").bind(shopName);
	}
	if(showOnlyOrderStatus)
	{
		if(orderStatus)
			query.where("not ordering_id is null");
		else
			query.where("ordering_id is null");
	}
	query.orderBy("user_id");
*/
