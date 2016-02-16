#include <Wt/WPanel>
#include <Wt/WText>
#include <string>
using std::string;

class ItemGroupCheckbox : public Wt::WPanel
{
public:
	ItemGroupCheckbox(string shopName, Wt::WContainerWidget *parent = NULL);
private:
	Wt::WText *totalField;
	double total;
	std::set<int> selectedUserIds;
};
