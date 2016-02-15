#include "BasketFormTemplate.hpp"

#include <boost/foreach.hpp>
#include <Wt/WText>

BasketFormTemplate::BasketFormTemplate(bpt::ptree &pt)
:item_template(pt.get<string>("item_template")),
form_template(pt.get<string>("form_template"))
{
}


Wt::WWidget* BasketFormTemplate::getBasketAddWidget(std::set<PItem> items) const
{
	std::stringstream ss;
	size_t number=0;
	BOOST_FOREACH(PItem item,items)
	{
		ss << item->formatInto(item_template, number);
		number++;
	}
	string html=str(boost::format(form_template) % ss.str());
	Wt::WText *text=new Wt::WText(html, Wt::XHTMLUnsafeText);
	return text;
}
