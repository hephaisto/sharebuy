# sharebuy
Share online-orders with friends to save on shipping

# Features
* web interface with login and registration
* parsing of shop URLs and corresponding pages for the following attributes
 * article number
 * article name
 * price
* per-user total price calculation for orders
* put all items from an order to the shop's cart with one click
* customizable for shops via regexes if the following conditions are met:
 * There is a html form for direct item entry on the shop's website
 * The article number for the above mentioned shop is available somewhere on the page that is reachable by the pasted URL

# Limitations
* Unfortunately not all shops follow the above-mentioned conditions
* Prices etc. are only parsed on item-adding. Price changes will not be detected!
* No support for complex pages (e.g. different versions of one item selectable via dropdown)

# Installation
## Runtime dependencies
* libwt, libwtdbosqlite
* several boost libraries (filesystem, regex, log, thread, system, program options)

## Build dependencies
* scons

# compilation

    scons

# Usage
    Release/sharebuy --shopdir <path/to/shop/config> --dbfile <path/to/file/for/database> --docroot /usr/share/Wt/ --http-address <interface> --http-port <port>
Both shopdir and dbfile have default values suitable for running directly from the dev tree.
