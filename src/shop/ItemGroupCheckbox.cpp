#include "ItemGroupCheckbox.hpp"

#include <boost/foreach.hpp>
#include <Wt/WCheckBox>
#include <Wt/WTable>
#include <Wt/WPushButton>

#include "../application.hpp"

ItemGroupCheckbox::ItemGroupCheckbox(string shopName, Wt::WContainerWidget *parent)
:Wt::WPanel(parent),
total(0)
{
	Wt::WContainerWidget *root = new Wt::WContainerWidget();
	setCentralWidget(root);

	Session& dbSession = static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession;
	dbo::Transaction transaction(dbSession);
	// user_id, item count, total price
	typedef boost::tuple<int, string, int, double> QueryTuple;
	dbo::Query<QueryTuple> query = dbSession.query<QueryTuple>(
		"select auth_info.user_id as userID, auth_identity.identity as name, sum(item.count) as count, sum(item.count*item.price) as total "
		"from auth_identity, auth_info, item "
		"where auth_identity.auth_info_id = auth_info.id "
		"and auth_info.user_id = item.user_id "
		"and item.shop_name = ? "
		"and item.ordering_id is null "
		"group by userID"
		).bind(shopName);

	dbo::collection<QueryTuple> result = query; // execute query

	Wt::WTable *table = new Wt::WTable(root);
	table->setHeaderCount(1);
	table->setWidth(Wt::WLength("50%"));

	table->elementAt(0, 0)->addWidget(new Wt::WText("user"));
	table->elementAt(0, 1)->addWidget(new Wt::WText("items"));
	table->elementAt(0, 2)->addWidget(new Wt::WText("total"));
	table->elementAt(0, 3)->addWidget(new Wt::WText("order for this user"));

	totalField = new Wt::WText("0.00");
	
	Wt::WPushButton *btnOrder = new Wt::WPushButton("order");
	btnOrder->setEnabled(false);
	btnOrder->clicked().connect([ this, &dbSession, shopName ](Wt::WMouseEvent)
	{
		dbo::Transaction transaction(dbSession);
		dbo::ptr<Order> order = dbSession.add(new Order());
		order.modify()->user = dbSession.user();
		transaction.commit();


		dbo::Transaction transaction2(dbSession);
		BOOST_FOREACH(int userId, selectedUserIds)
		{
			dbSession.execute("update item set ordering_id = ? where item.user_id = ? and item.ordering_id is null and item.shop_name = ? ").bind(order.id()).bind(userId).bind(shopName);
		}
		
		Wt::WApplication::instance()->setInternalPath("/user/orders", true);
	});

	size_t row=1;
	BOOST_FOREACH(auto tuple, result)
	{
		int userId = tuple.get<0>();
		double userTotal = tuple.get<3>();
		Wt::WCheckBox *checkbox = new Wt::WCheckBox();
		checkbox->changed().connect([this, checkbox, userId, userTotal, btnOrder](Wt::NoClass)
		{
			if(checkbox->checkState() == Wt::Checked)
			{
				selectedUserIds.insert(userId);
				total += userTotal;
				btnOrder->setEnabled(true);
			}
			else
			{
				selectedUserIds.erase(userId);
				total -= userTotal;
				if(selectedUserIds.empty())
					btnOrder->setEnabled(false);
			}
			totalField->setText(boost::lexical_cast<string>(total));
		});
		table->elementAt(row,3)->addWidget(checkbox);
		table->elementAt(row,0)->addWidget(new Wt::WText(tuple.get<1>()));
		table->elementAt(row,1)->addWidget(new Wt::WText(boost::lexical_cast<string>(tuple.get<2>())));
		table->elementAt(row,2)->addWidget(new Wt::WText(boost::lexical_cast<string>(tuple.get<3>())));

		/*
		std::cout<<"userID: "<<tuple.get<0>()<<"\n";
		std::cout<<"username: "<<tuple.get<1>()<<"\n";
		std::cout<<"count: "<<tuple.get<2>()<<"\n";
		std::cout<<"total: "<<tuple.get<3>()<<"\n";
		*/

		row++;
	}
	table->elementAt(row,2)->addWidget(totalField);
	table->elementAt(row,3)->addWidget(btnOrder);

	}

/*std::set<string> ItemGroupCheckbox::selectedUserIds()
{
}*/
