#include <iostream>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include "application.hpp"
#include "shop/Shop.hpp"


int main(int argc, char **argv)
{
	namespace po = boost::program_options;
	po::options_description desc("Basic options");
	desc.add_options()
		("help", "print this help message")
		("shopdir", po::value<string>()->default_value("shops"), "directory in which shop definitions are stored")
		("dbfile", po::value<string>()->default_value("sharebuy.db"), "sqlite file for data storage")
	;
	po::parsed_options parsed = po::command_line_parser(argc, argv).options(desc).allow_unregistered().run();
	po::variables_map args;
	po::store(parsed, args);

	if(args.count("help"))
	{
		std::cout<<desc<<"\n";
		return Wt::WRun(argc, argv);
	}

	string shopConfigDir(args["shopdir"].as<string>());
	string databaseFile(args["dbfile"].as<string>());

	try
	{
		shared_ptr<std::map<string, shared_ptr<Shop> > > shops=loadShops(shopConfigDir);
		if(shops->empty())
			throw ShopLoaderException("unable to load any shop!");
		Session::configureAuth();
		return Wt::WRun(argc, argv, [&](const Wt::WEnvironment& env){return new ShareBuy(env, shops, databaseFile);});
	}
	catch(ShopLoaderException &e)
	{
		BOOST_LOG_TRIVIAL(fatal)<<e.what();
		return 1;
	}
}
