#ifndef H_SHAREBUY_USER
#define H_SHAREBUY_USER

#include <Wt/Dbo/Dbo>
#include <Wt/WGlobal>
#include <Wt/Auth/Dbo/AuthInfo>
namespace dbo = Wt::Dbo;

class User;
typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;

#include "../shop/Item.hpp"
#include "../shop/Order.hpp"

class User
{
public:
	dbo::collection<dbo::ptr<Item> > items;
	dbo::collection<dbo::ptr<Order> > orders;
	Wt::Dbo::collection< Wt::Dbo::ptr<AuthInfo> > authInfos;

	template<class Action>
	void persist(Action& a)
	{
		dbo::hasMany(a, items, dbo::ManyToOne, "user");
		dbo::hasMany(a, orders, dbo::ManyToOne, "user");
		dbo::hasMany(a, authInfos, Wt::Dbo::ManyToOne, "user");
	}

	string getUsername() const;
};

DBO_EXTERN_TEMPLATES(User);

#endif // H_SHAREBUY_USER
