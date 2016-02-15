#include "Item.hpp"

Item::Item()
{
}

Item::Item(const string shop_name, const string shop_specific_id, const string shop_specific_id_2, const string title, const string url, double price, const int count)
:shop_name(shop_name),
shop_specific_id(shop_specific_id),
shop_specific_id_2(shop_specific_id_2),
title(title),
url(url),
price(price),
count(count)
{
}

string Item::formatInto(const boost::format fmt, int number) const
{
	return (boost::format(fmt) % number % shop_specific_id % shop_specific_id_2 % count).str();
}
