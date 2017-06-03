#include "Shop.hpp"
#include <boost/filesystem.hpp>
namespace bf=boost::filesystem;
#include <iostream>
#include <boost/log/trivial.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/foreach.hpp>

#include "BasketFormTemplate.hpp"

Shop::Shop(bpt::ptree &pt)
:name(pt.get<string>("name")),
basketAdd(std::make_shared<BasketFormTemplate>(pt.get_child("basket_form"))),
inputURLRegex{pt.get<string>("input_url_regex")},
pageRegex{pt.get<string>("page_regex")},
imageTemplate{pt.get<string>("item_image_template")}
{
	imageTemplate.exceptions(boost::io::all_error_bits & ~boost::io::too_many_args_bit); // not all parameters needed in all predefined expressions
}

ShopList loadShops(string shopConfigDir)
{
	BOOST_LOG_TRIVIAL(info)<<"loading shops from "<<shopConfigDir;
	ShopList result(new std::map<string, shared_ptr<Shop> >);
	bf::path shop_dir(shopConfigDir);

	if(!bf::exists(shop_dir))
		throw ShopLoaderException("shop directory not found");

	if(!bf::is_directory(shop_dir))
		throw ShopLoaderException("shop path not a directory");

	for(bf::directory_iterator it=bf::directory_iterator(shop_dir);it!=bf::directory_iterator();it++)
	{
		try
		{
			BOOST_LOG_TRIVIAL(info)<<"loading shop "<<*it;
			bpt::ptree pt;
			bpt::read_info(bf::path(*it).string(),pt);
			shared_ptr<Shop> shop = std::make_shared<Shop>(pt);
			result->insert(std::make_pair(shop->name, shop));
		}
		catch(std::exception &e)
		{
			BOOST_LOG_TRIVIAL(error)<<e.what();
		}
	}
	BOOST_LOG_TRIVIAL(info)<<"finished loading shops";
	return result;
}


ShopLoaderException::ShopLoaderException(string text)
:text(text)
{
}

const char* ShopLoaderException::what() const noexcept
{
	return text.c_str();
}



/*
   if (exists(p))    // does p actually exist?
    {
      if (is_regular_file(p))        // is p a regular file?
        cout << p << " size is " << file_size(p) << '\n';

      else if (is_directory(p))      // is p a directory?
      {
        cout << p << " is a directory containing:\n";

        copy(directory_iterator(p), directory_iterator(), // directory_iterator::value_type
          ostream_iterator<directory_entry>(cout, "\n")); // is directory_entry, which is
                                                          // converted to a path by the
                                                          // path stream inserter
      }

      else
        cout << p << " exists, but is neither a regular file nor a directory\n";
    }
    else
      cout << p << " does not exist\n";
  }

  catch (const filesystem_error& ex)
  {
    cout << ex.what() << '\n';
  }

  return 0;
  */
