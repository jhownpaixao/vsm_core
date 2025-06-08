modded class VicinitySlotsContainer
{
	override void MouseClick( Widget w, int x, int y, int button)
	{
		super.MouseClick(w, x, y, button);
		string name = w.GetName();
		name.Replace( "PanelWidget", "Render" );
		ItemPreviewWidget item_preview = ItemPreviewWidget.Cast( w.FindAnyWidget( name ) );
		EntityAI item = item_preview.GetItem();
		InventoryItem itemAtPos = InventoryItem.Cast( item );
		
		if (button == MouseState.RIGHT)
		{
			if (itemAtPos)
				ShowActionMenu(itemAtPos);	
		}
	}
}
