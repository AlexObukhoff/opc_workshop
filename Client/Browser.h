#pragma once

class OPCClient;

///  ласс отвечающий за перечисление узлов в дереве 
class BrowseItems
{
	string browsePosition;
	OPCClient *client;
	BrowseItems( OPCClient *cl ) : client(cl)
	{
		
	}
	friend class OPCClient;

public:

	BrowseItems();
	BrowseItems( OPCClient *cl, string name );
	BrowseItems( const BrowseItems& b);
	const BrowseItems& operator =( const BrowseItems& b);

	struct Item
	{
		string name;
		string itemId;
		unsigned flag;
	};
	std::vector<Item> items;

	void Browse( string name );

	size_t size()
	{
		return items.size();
	}

private:
	static void Browse( BrowseItems &it, IOPCBrowseServerAddressSpace* browse, string name );
	static void Browse( BrowseItems &it, IOPCBrowse* browse, string name );
};