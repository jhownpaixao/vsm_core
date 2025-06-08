/* modded class AttachmentCategoriesRow
{
	
    override void MouseClick(Widget w, int x, int y, int button)
	{
		super.MouseClick(w, x, y, button);
        Print("AttachmentCategoriesRow::MouseClick: w = " + w + ", x = " + x + ", y = " + y + ", button = " + button);
		ItemBase selectedItem;
		SlotsIcon icon; 
		w.GetUserData(icon);
		if (icon)
			selectedItem = ItemBase.Cast(icon.GetEntity());

		if (selectedItem)
		{
			if (button == MouseState.RIGHT)
			{
            	Print("AttachmentCategoriesRow::MouseClick: g_Game.IsLeftCtrlDown()");
				ShowActionMenu(selectedItem);	
			}
		}
	}
}
 */