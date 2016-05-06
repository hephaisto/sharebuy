#include <Wt/WContainerWidget>

#include "User.hpp"

class UserDisplay : public Wt::WContainerWidget
{
public:
	UserDisplay(PUser user, bool adminMode, Wt::WContainerWidget *parent = NULL);
};
