#ifndef H_ITEM
#define H_ITEM

#include <string>
using std::string;

#include <boost/format.hpp>

#include <Wt/Dbo/Dbo>
namespace dbo = Wt::Dbo;

class Item;

#include "../user/User.hpp"
#include "Order.hpp"

class Item
{
public:
	string shop_name;
	string shop_specific_id;
	string shop_specific_id_2;
	string title;
	string url;
	double price;
	int count;
	dbo::ptr<User> user;
	dbo::ptr<Order> order;

	Item();
	Item(const string shop_name, const string shop_specific_id, const string shop_specific_id_2, const string title, const string url, double price, const int count);

	string formatInto(const boost::format fmt, int number) const;

	template<class Action>
	void persist(Action& a)
	{
		dbo::field(a, shop_name, "shop_name");
		dbo::field(a, shop_specific_id, "shop_specific_id");
		dbo::field(a, shop_specific_id_2, "shop_specific_id_2");
		dbo::field(a, title, "title");
		dbo::field(a, url, "url");
		dbo::field(a, price, "price");
		dbo::field(a, count, "count");
		dbo::belongsTo(a, user, "user");
		dbo::belongsTo(a, order, "ordering", dbo::OnDeleteSetNull);
	}
};

typedef dbo::ptr<Item> PItem;
typedef dbo::collection<PItem> PItems;

#endif //defined H_ITEM
