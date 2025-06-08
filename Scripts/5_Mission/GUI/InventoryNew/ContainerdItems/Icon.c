/* modded class Icon
{
	override void MouseClick(Widget w, int x, int y, int button)
	{
		super.MouseClick(w, x, y, button);

		if (button == MouseState.RIGHT)
		{
			Print("Icon::MouseClick: button == MouseState.RIGHT");
			if (m_Item)
			{
				Print("Icon::MouseClick: GetDayZGame().IsLeftCtrlDown()");
				ShowActionMenu(m_Item);
			}	
		}
	}
} */