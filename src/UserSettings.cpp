#include "UserSettings.hpp"

#include <Wt/WText>
#include <Wt/Auth/UpdatePasswordWidget>
#include <Wt/WApplication>
#include <Wt/WTextArea>
#include <Wt/WPanel>

#include "user/Session.hpp"
#include "application.hpp"

UserSettings::UserSettings(Wt::WContainerWidget *parent)
:Wt::WContainerWidget(parent)
{
	new Wt::WText("<h1>settings</h1>", this);
	
	Session& dbSession = static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession;
	const Wt::Auth::User &user = dbSession.login().user();

	Wt::WPanel *passwordPanel = new Wt::WPanel(this);
	passwordPanel->setTitle("Change Password");
	Wt::WWidget* changePassword=static_cast<ShareBuy*>(Wt::WApplication::instance())->getAuthWidget()->createUpdatePasswordView(user, false);
	passwordPanel->setCentralWidget(changePassword);

	Wt::WPanel *aboutPanel = new Wt::WPanel(this);
	aboutPanel->setTitle("More info about yourself (about)");
	Wt::WTextArea *ta = new Wt::WTextArea();
	ta->changed().connect(std::bind([&dbSession, ta]()
	{
		dbo::Transaction transaction(dbSession);
		dbSession.user().modify()-> about = ta->text().toUTF8();
	}));
	aboutPanel->setCentralWidget(ta);
}
