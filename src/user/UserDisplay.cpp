#include "UserDisplay.hpp"

#include <Wt/WTable>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>

#include "Session.hpp"
#include "../application.hpp"

UserDisplay::UserDisplay(PUser user, bool adminMode, Wt::WContainerWidget *parent)
:Wt::WContainerWidget(parent)
{
	Wt::WTable *table = new Wt::WTable(this);
	table->setStyleClass("table table-striped");

	int row = 0;

	table->elementAt(row, 0)->addWidget(new Wt::WText("name"));
	table->elementAt(row, 1)->addWidget(new Wt::WText(user->getUsername()));
	row++;

	table->elementAt(row, 0)->addWidget(new Wt::WText("rank"));
	table->elementAt(row, 1)->addWidget(new Wt::WText(user->isAdmin?"admin":"normal"));
	row++;

	table->elementAt(row, 0)->addWidget(new Wt::WText("about"));
	table->elementAt(row, 1)->addWidget(new Wt::WText(user->about));
	row++;

	if(adminMode)
	{
		table->elementAt(row, 0)->addWidget(new Wt::WText("reset password"));
		Wt::WLineEdit *newPassword = new Wt::WLineEdit();
		Wt::WPushButton *newPasswordBtn = new Wt::WPushButton("Set new password");
		newPasswordBtn->clicked().connect(std::bind([user, newPassword]()
		{
			Session& dbSession = static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession;
			dbo::Transaction transaction(dbSession);
			user.modify()->updatePassword(newPassword->text());
		}));
		table->elementAt(row, 1)->addWidget(newPassword);
		table->elementAt(row, 1)->addWidget(newPasswordBtn);
		row++;
	}
}
