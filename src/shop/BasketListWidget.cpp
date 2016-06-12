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



BasketListWidget::BasketListWidget(ShopList shops, string shopName, PUser user, string orderId, Wt::WContainerWidget *parent)
:Wt::WPanel(parent),
shops(shops),
shopName(shopName),
user(user),
orderId(orderId),
canDelete(false),
orderStatus(false),
showOnlyOrderStatus(false),
showAddToCartButton(false)
{
	setCollapsible(true);
	update();
}

void BasketListWidget::setOnlyOrderStatus(bool orderStatus)
{
	showOnlyOrderStatus=true;
	this->orderStatus=orderStatus;
}

/*
BasketListWidget::BasketListWidget(ShopList shops, dbo::Query<PItem> query, Wt::WContainerWidget *parent)
:Wt::WContainerWidget(parent),
shops(shops),
query(query),
canDelete(false)
{
	update();
}
*/

void BasketListWidget::setCanDelete(bool canDelete)
{
	this->canDelete=canDelete;
}

void BasketListWidget::setShowAddToCartButton(bool showAddToCartButton)
{
	this->showAddToCartButton = showAddToCartButton;
}

void BasketListWidget::update()
{
	Wt::WContainerWidget *root = new Wt::WContainerWidget();
	setCentralWidget(root); // delete old content
	Session& dbSession = static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession;
	dbo::Transaction transaction(dbSession);
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

	PItems database_items = query; // execute query
	std::set<PItem> items(database_items.begin(),database_items.end());

	/*Wt::WPushButton *btnCollapse = new Wt::WPushButton("+", titleBarWidget());
	btnCollapse->clicked().connect([btnCollapse,this](Wt::WMouseEvent event)
	{
		setCollapsed(!isCollapsed());
	});*/

	/*if(showOnlyOrderStatus && orderStatus==false) // only orderable items shown?
	{
		Wt::WPushButton *btn = new Wt::WPushButton("order", titleBarWidget());
	}*/

	Wt::WTable *table = new Wt::WTable(root);
	table->setHeaderCount(1);
	table->setWidth(Wt::WLength("100%"));

	table->elementAt(0, 0)->addWidget(new Wt::WText(""));
	table->elementAt(0, 1)->addWidget(new Wt::WText("Number"));
	table->elementAt(0, 2)->addWidget(new Wt::WText("Title"));
	table->elementAt(0, 3)->addWidget(new Wt::WText("Count"));
	table->elementAt(0, 4)->addWidget(new Wt::WText("Price"));
	table->elementAt(0, 5)->addWidget(new Wt::WText("Total"));


	size_t number=0;
	std::map<dbo::ptr<User>, double> userSums;
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
		if( canDelete )
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

	//if(userId == "") // show per-user financial overview
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

	std::cerr<<"item!"<<showAddToCartButton<<"\n";;
	if( (showAddToCartButton) && !(items.empty()) )
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
