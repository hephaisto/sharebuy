#include "BasketAddTemplate.hpp"
#include <boost/property_tree/ptree.hpp>
namespace bpt=boost::property_tree;
#include <boost/format.hpp>

class BasketFormTemplate : public BasketAddTemplate
{
public:
	BasketFormTemplate(bpt::ptree &pt);
	virtual Wt::WWidget* getBasketAddWidget(std::set<PItem> items) const;
private:
	const boost::format form_template;
	const boost::format item_template;
};

