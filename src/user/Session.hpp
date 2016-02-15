#ifndef H_SHAREBUY_SESSION
#define H_SHAREBUY_SESSION


#include <Wt/Dbo/Session>
#include <Wt/Dbo/ptr>
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Auth/Login>
#include <Wt/Auth/PasswordService>

#include "User.hpp"

namespace dbo = Wt::Dbo;

typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class Session : public dbo::Session
{
public:
	Session(const std::string& sqliteDb);
	virtual ~Session();

	Wt::Auth::AbstractUserDatabase& users();
	Wt::Auth::Login& login() { return login_; }

	static void configureAuth();
	static const Wt::Auth::AuthService& auth();
	static const Wt::Auth::PasswordService& passwordAuth();
	static const std::vector<const Wt::Auth::OAuthService *>& oAuth();
	dbo::ptr<User> user();
private:
	dbo::backend::Sqlite3 connection_;
	UserDatabase *users_;
	Wt::Auth::Login login_;
};

#endif
