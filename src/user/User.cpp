#include "User.hpp"
#include <Wt/Dbo/Impl>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/Auth/Identity>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/PasswordHash>
#include <Wt/Auth/HashFunction>

#include "Session.hpp"

DBO_INSTANTIATE_TEMPLATES(User);

string User::getUsername() const
{
	return (*authInfos.begin())->identity(Wt::Auth::Identity::LoginName).toUTF8();
}

void User::updatePassword(Wt::WString password)
{
	Wt::Auth::PasswordVerifier verifier;
	verifier.addHashFunction(new Wt::Auth::BCryptHashFunction(7));
	Wt::Auth::PasswordHash ph = verifier.hashPassword(password);
	(*authInfos.begin()).modify()->setPassword(ph.value(), ph.function(), ph.salt());
}
