#ifndef H_SHAREBUY_SHOP
#define H_SHAREBUY_SHOP

#include <string>
using std::string;
#include <memory>
#include <set>
using std::shared_ptr;
#include <boost/property_tree/ptree.hpp>
namespace bpt=boost::property_tree;
#include <boost/regex.hpp>

#include "BasketAddTemplate.hpp"

class Shop
{
public:
	explicit Shop(bpt::ptree &pt);
	const string name;
	const shared_ptr<const BasketAddTemplate> basketAdd;
	const boost::regex inputURLRegex;
	boost::format imageTemplate;
	const boost::regex pageRegex;
};

typedef shared_ptr<std::map<string, shared_ptr<Shop> > > ShopList;
ShopList loadShops(string shopConfigFolder);



class ShopLoaderException : public std::exception
{
public:
	explicit ShopLoaderException(string text);
	virtual const char* what() const noexcept;
private:
	string text;
};

#endif // H_SHAREBUY_SHOP
