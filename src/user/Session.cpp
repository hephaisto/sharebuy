#include "Session.hpp"

#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/Auth/Dbo/UserDatabase>
#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/OAuthService>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/HashFunction>

#include "User.hpp"
#include "../shop/Item.hpp"

Session::Session(const std::string& sqliteDb)
:connection_(sqliteDb)
{
	connection_.setProperty("show-queries", "true");
	setConnection(connection_);

	mapClass<User>("user");
	mapClass<AuthInfo>("auth_info");
	mapClass<AuthInfo::AuthIdentityType>("auth_identity");
	mapClass<AuthInfo::AuthTokenType>("auth_token");

	mapClass<Item>("item");
	mapClass<Order>("ordering");

	try
	{
		std::cerr << "Creating database...\n";
		createTables();
		std::cerr << "Created database.\n";
	}
	catch (Wt::Dbo::Exception& e)
	{
		std::cerr << e.what();
		std::cerr << "Using existing database\n";
	}

	users_ = new UserDatabase(*this);
}

Session::~Session()
{
delete users_;
}

Wt::Auth::AbstractUserDatabase& Session::users()
{
return *users_;
}


namespace {
  Wt::Auth::AuthService myAuthService;
  Wt::Auth::PasswordService myPasswordService(myAuthService);
  std::vector<const Wt::Auth::OAuthService *> myOAuth;
}


dbo::ptr<User> Session::user()
{
	if (login_.loggedIn())
	{
		dbo::ptr<AuthInfo> authInfo = users_->find(login_.user());
		dbo::ptr<User> user = authInfo->user();
		if(!user)
		{
			user = add(new User());
			authInfo.modify()->setUser(user);
		}
		return user;
	}
	else
		return dbo::ptr<User>();
}

void Session::configureAuth()
{
	myAuthService.setAuthTokensEnabled(true, "logincookie");
	myAuthService.setAuthTokenValidity(365*24*60);
	myAuthService.setEmailVerificationEnabled(true);

	Wt::Auth::PasswordVerifier *verifier = new Wt::Auth::PasswordVerifier();
	verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
	myPasswordService.setVerifier(verifier);
	myPasswordService.setAttemptThrottlingEnabled(true);

	/*
	if (Wt::Auth::GoogleService::configured())
		myOAuthServices.push_back(new Wt::Auth::GoogleService(myAuthService));
	*/
}

const Wt::Auth::AuthService& Session::auth()
{
  return myAuthService;
}

const Wt::Auth::PasswordService& Session::passwordAuth()
{
  return myPasswordService;
}

const std::vector<const Wt::Auth::OAuthService *>& Session::oAuth()
{
  return myOAuth;
}
