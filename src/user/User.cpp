#include "User.hpp"
#include <Wt/Dbo/Impl>
#include <Wt/Auth/Dbo/AuthInfo>
#include <Wt/Auth/Identity>

DBO_INSTANTIATE_TEMPLATES(User);

string User::getUsername() const
{
	return (*authInfos.begin())->identity(Wt::Auth::Identity::LoginName).toUTF8();
}
