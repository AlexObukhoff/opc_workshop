#pragma once

#include <string>
#include <vector>

class OPCClient;

/// class for enumerating items in tags tree
class BrowseItems
{
	std::string browsePosition;
	OPCClient *client;
	BrowseItems( OPCClient *cl ) : client(cl)
	{
		
	}
	friend class OPCClient;

public:

	BrowseItems();
	BrowseItems( OPCClient *cl, std::string name );
	BrowseItems( const BrowseItems& b);
	const BrowseItems& operator =( const BrowseItems& b);

	struct Item
	{
		std::string name;
		std::string itemId;
		unsigned flag;
	};
	std::vector<Item> items;

	void Browse( std::string name );

	size_t size()
	{
		return items.size();
	}

private:
	static void Browse( BrowseItems &it, IOPCBrowseServerAddressSpace* browse, std::string name );
	static void Browse( BrowseItems &it, IOPCBrowse* browse, std::string name );
};