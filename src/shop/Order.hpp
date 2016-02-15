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

	template<class Action>
	void persist(Action& a)
	{
		dbo::belongsTo(a, user, "user");
		dbo::hasMany(a, items, dbo::ManyToOne, "ordering");
	}
};


#endif //defined H_ORDER
