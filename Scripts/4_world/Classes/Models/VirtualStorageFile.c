class VirtualStorageFile
{
	string virtualId = "";
	ref array<ref VirtualObjectContext> storedItems = { };
};

class VirtualObjectContext
{
	string virtualId = "";
	string contextFile = "";
}