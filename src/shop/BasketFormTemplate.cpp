#include "BasketFormTemplate.hpp"

#include <boost/foreach.hpp>
#include <Wt/WText>

BasketFormTemplate::BasketFormTemplate(bpt::ptree &pt)
:item_template(pt.get<string>("item_template")),
form_template(pt.get<string>("form_template"))
{
	// not all parameters needed in all predefined expressions
	item_template.exceptions(boost::io::all_error_bits & ~boost::io::too_many_args_bit);
	form_template.exceptions(boost::io::all_error_bits & ~boost::io::too_many_args_bit);
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
