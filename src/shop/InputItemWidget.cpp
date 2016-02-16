#include "InputItemWidget.hpp"

#include <Wt/WLineEdit>
#include <iostream>
#include <boost/foreach.hpp>
#include <Wt/Utils>
#include <Wt/WIntValidator>
#include <Wt/WPushButton>
#include <Wt/Http/Client>
#include <Wt/Http/Message>
#include <Wt/WApplication>
#include <boost/log/trivial.hpp>
#include <fstream>
#include <limits>

#include "../application.hpp"

/*
class ShopParseException : public std::exception
{
public:
	ShopParseException
};*/

InputItemWidget::InputItemWidget(ShopList shops, Wt::WContainerWidget *parent)
:Wt::WContainerWidget(parent),
shops(shops)
{
	edit=new Wt::WLineEdit(this);
	edit->setPlaceholderText("Paste URL here");

	countEdit=new Wt::WLineEdit(this);
	Wt::WIntValidator *val = new Wt::WIntValidator();
	val->setBottom(0);
	val->setMandatory(true);
	countEdit->setValidator(val);
	countEdit->setPlaceholderText("count");

	btn = new Wt::WPushButton("Add", this);
	btn->setStyleClass("btn-primary");
	btn->clicked().connect(this,&InputItemWidget::changed);
}

void InputItemWidget::changed()
{
	if(countEdit->validate()!=Wt::WValidator::Valid)
	{
		std::cout<<"invalid!\n";
		return;
	}
	int count = boost::lexical_cast<int>(countEdit->text());
	string newText = Wt::Utils::urlDecode(edit->text().toUTF8());
	std::cout<<"changed to "<<newText<<"\n";
	boost::smatch url_match;

	//boost::regex_search(edit->text().toUTF8(),m,shop->inputURLRegex);
	//boost::regex r{"ARTICLEID=(?<id>\\d+)"};
	//boost::regex r{re->text().toUTF8()};
	//for(auto shop=shops->begin();shop!=shops->end();shop++)
	bool found=false;
	BOOST_FOREACH(auto tuple, *shops)
	{
		auto shop=tuple.second;
		if(boost::regex_search(newText,url_match, shop->inputURLRegex))
		{
			found=true;
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
			Wt::Http::Client *client = new Wt::Http::Client(Wt::WApplication::instance());
			client->setTimeout(10);
			client->setMaximumResponseSize(1024*1024);
			client->done().connect([ this, client, newText, url_match, shop, count, urlMatches ](boost::system::error_code err, const Wt::Http::Message& response, Wt::NoClass, Wt::NoClass, Wt::NoClass, Wt::NoClass)
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

							BOOST_FOREACH(auto m, matches)
								std::cout<<"matches[\""<<m.first<<"\"]=\""<<m.second<<"\"\n";

							Session& dbSession = static_cast<ShareBuy*>(Wt::WApplication::instance())->dbSession;
							dbo::Transaction transaction(dbSession);
							PItem item = dbSession.add(new Item( shop->name, matches["number"], matches["number2"], matches["title"], newText, price, count));
							item.modify()->user = dbSession.user();
							transaction.commit();
							_itemEntered.emit();
							edit->setText("");
							countEdit->setText("");
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
				btn->setEnabled(true);
				btn->setText("Add");
				edit->setPlaceholderText("Paste URL here");
				Wt::WApplication::instance()->triggerUpdate();
			});
			if(client->get(edit->text().toUTF8()))
			{
				btn->setEnabled(false);
				btn->setText("adding...");
				std::cout<<"loading info from page...\n";
			}

		}
	}
	if(!found)
	{
		edit->setText("");
		edit->setPlaceholderText("invalid URL!");
	}
}

Wt::Signal<void>& InputItemWidget::itemEntered()
{
	return _itemEntered;
}
