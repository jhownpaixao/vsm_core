/* modded class ContainerWithCargoAndAttachments
{
	
    override void MouseClick2(Widget w, int x, int y, int button)
	{
		super.MouseClick2(w, x, y, button);

        Print("ContainerWithCargoAndAttachments::MouseClick2: w = " + w + ", x = " + x + ", y = " + y + ", button = " + button);
		SlotsIcon icon;
		w.GetUserData(icon);
		
		ItemBase selectedItem;
		if (icon)
			selectedItem = ItemBase.Cast(icon.GetEntity());
		
		if (selectedItem)
		{
			if (button == MouseState.RIGHT)
			{
				Print("ContainerWithCargoAndAttachments::MouseClick2: g_Game.IsLeftCtrlDown() - ShowActionMenu");
                ShowActionMenu(selectedItem);
			}
		}
	}
}
 */