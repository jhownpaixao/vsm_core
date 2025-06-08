/* modded class HandsContainer: Container
{
	override void MouseClick2(Widget w, int x, int y, int button)
	{
		super.MouseClick2(w, x, y, button);

		Print("HandsContainer::MouseClick2: w = " + w + ", x = " + x + ", y = " + y + ", button = " + button);
		string name = w.GetName();
		name.Replace("PanelWidget", "Render");
		ItemPreviewWidget itemPreview = ItemPreviewWidget.Cast(w.FindAnyWidget(name));
		ItemBase selectedItem = ItemBase.Cast(itemPreview.GetItem());
		
		if (selectedItem)
		{
			Print("HandsContainer::MouseClick2: g_Game.IsLeftCtrlDown() - ShowActionMenu");
			ShowActionMenu(selectedItem);	
		}
	}
} */
