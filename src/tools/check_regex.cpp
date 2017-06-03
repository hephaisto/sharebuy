#include <iostream>
#include <chrono>
#include <thread>
#include <boost/foreach.hpp>
#include <Wt/Utils>
#include <Wt/WIntValidator>
#include <Wt/WPushButton>
#include <Wt/Http/Client>
#include <Wt/Http/Message>
#include <Wt/WIOService>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/filesystem.hpp>
namespace bf=boost::filesystem;
#include <fstream>
#include <limits>

#include "../shop/Shop.hpp"

using std::string;

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		std::cerr<< "USAGE: "<< argv[0] << "shopfile url\n";
		return 1;
	}

	bpt::ptree pt;
	bpt::read_info(bf::path(argv[1]).string(),pt);
	shared_ptr<Shop> shop = std::make_shared<Shop>(pt);

	string newText = Wt::Utils::urlDecode(argv[2]);
	std::cout<<"changed to "<<newText<<"\n";
	boost::smatch url_match;
	if(!boost::regex_search(newText,url_match, shop->inputURLRegex))
	{
		std::cerr << "url does not match supplied regex\n";
		return 3;
	}

	Wt::WIOService service;
	service.start();
	service.initializeThread();

	std::cout<<"match for shop "<<shop->name<<"\n";
	std::cout<<"number: "<<url_match["number"]<<"\n";
	std::cout<<"number2: "<<url_match["number2"]<<"\n";

	std::map<string, string> urlMatches;
	if(url_match["number"].matched)
		urlMatches["number"]=url_match["number"];
	if(url_match["number2"].matched)
		urlMatches["number2"]=url_match["number2"];
	if(url_match["title"].matched)
		urlMatches["title"]=url_match["title"];
	
	// get data from page
	Wt::Http::Client *client = new Wt::Http::Client(service);
	client->setTimeout(10);
	client->setMaximumResponseSize(1024*1024);
	client->done().connect([ client, newText, url_match, shop, urlMatches ](boost::system::error_code err, const Wt::Http::Message& response, Wt::NoClass, Wt::NoClass, Wt::NoClass, Wt::NoClass)
	{
		std::cout<<"done()\n";

		if(!err)
		{
			if(response.status() == 200)
			{
				std::cout<<"shop regexes:\n";
				boost::smatch pageMatch;
				if(boost::regex_search(response.body(), pageMatch, shop->pageRegex))
				{
					std::map<string, string> matches = urlMatches;

					// number
					if(pageMatch["number"].matched)
					{
						matches["number"]=pageMatch["number"];
					}

					// number2
					if(pageMatch["number2"].matched)
					{
						matches["number2"]=pageMatch["number2"];
					}

					// price
					if(pageMatch["price"].matched)
					{
						matches["price"]=pageMatch["price"];
					}

					// title
					if(pageMatch["title"].matched)
					{
						matches["title"]=pageMatch["title"];
					}

					BOOST_FOREACH(auto m, matches)
						std::cout<<"matches[\""<<m.first<<"\"]=\""<<m.second<<"\"\n";

					double price=std::numeric_limits<double>::quiet_NaN();
					try
					{
						price = boost::lexical_cast<double>(matches["price"]);
					}
					catch(boost::bad_lexical_cast &)
					{
						std::cerr<<"unable to convert \""<<matches["price"]<<"\" to float\n";
						try // try to fix "," as a comma separator
						{
							matches["price"].replace(matches["price"].find(','),1,{'.'});
							std::cerr<<"trying with \""<<matches["price"]<<"\"\n";
							price = boost::lexical_cast<double>(matches["price"]);
						}
						catch(boost::bad_lexical_cast &)
						{}
					}

					std::cout<<"Success!\n";
					// TODO print parsed results

				}
				else
				{
					BOOST_LOG_TRIVIAL(fatal)<<"unable to match page regex to shop response for shop "<<shop->name;
					std::ofstream responseFile("unmatched_response");
					responseFile<<response.body();
				}
			}
			else
				std::cout<<"response statuscode: "<<response.status()<<"\n";
		}
		else
			std::cout<<"error: "<<err.message()<<"\n";
		delete client;
	});
	if(!client->get(argv[2]))
	{
		std::cerr<<"unable to start client\n";
		return 4;
	}
	//std::cout<<"waiting...\n";
	//std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	service.stop();
	std::cout<<"end\n";
}

