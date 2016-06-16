#ifndef H_ORDER
#define H_ORDER

#include <Wt/Dbo/Dbo>
namespace dbo = Wt::Dbo;

class Order;

#include "../user/User.hpp"
#include "Item.hpp"

enum OrderStatus
{
	Ordered,
	Sent,
	Received
};

class Order
{
public:

	dbo::ptr<User> user;
	dbo::collection<dbo::ptr<Item> > items;
	Wt::WDateTime orderTime;
	Wt::WDateTime receiveTime;

	template<class Action>
	void persist(Action& a)
	{
		dbo::field(a, orderTime, "orderTime");
		dbo::field(a, receiveTime, "receiveTime");
		dbo::belongsTo(a, user, "user");
		dbo::hasMany(a, items, dbo::ManyToOne, "ordering");
	}
};

typedef dbo::ptr<Order> POrder;
typedef dbo::collection<POrder> POrders;


#endif //defined H_ORDER
