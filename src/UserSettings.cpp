#include "UserSettings.hpp"

#include <Wt/WText>
#include <Wt/Auth/UpdatePasswordWidget>
#include <Wt/WApplication>

#include "user/Session.hpp"
#include "application.hpp"

UserSettings::UserSettings(Wt::WContainerWidget *parent)
:Wt::WContainerWidget(parent)
{
	new Wt::WText("settings", this);
	Session& dbSession = static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession;
	const Wt::Auth::User &user = dbSession.login().user();
	Wt::WWidget* changePassword=static_cast<ShareBuy*>(Wt::WApplication::instance())->getAuthWidget()->createUpdatePasswordView(user, false);
	addWidget(changePassword);
	//new Wt::Auth::UpdatePasswordWidget(user, NULL/*registrationModel*/, NULL, this);

	//this->addWidget(Wt::Auth::AuthWidget::createUpdatePasswordView(user, false));
}
