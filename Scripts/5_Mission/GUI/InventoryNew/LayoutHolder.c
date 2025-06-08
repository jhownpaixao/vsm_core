modded class LayoutHolder extends ScriptedWidgetEventHandler
{
	private autoptr VSM_ContextMenu m_context_menu;

	VSM_ContextMenu GetContextMenu()
	{
		if(!m_context_menu)
		{
			m_context_menu = new VSM_ContextMenu;
			m_context_menu.Init(GetGame().GetUIManager().GetMenu().GetLayoutRoot());
		}

		return m_context_menu;
	}

	override void ShowActionMenu(InventoryItem item)
	{
		PlayerBase m_player = PlayerBase.Cast( GetGame().GetPlayer() );
		HideOwnedTooltip();
		m_am_entity1 = item;
		m_am_entity2 = null;
		VSM_ContextMenu cmenu = GetContextMenu();

		cmenu.Hide();
		cmenu.Clear();

		if (m_am_entity1 == null)
			return;

		ItemBase itemBase = ItemBase.Cast(m_am_entity1);
		if (!itemBase || !itemBase.VSM_IsVirtualStorage())
			return;
		
		array<ActionBase_Basic> actions;
		itemBase.GetActions(InteractActionInput, actions);
		if (actions)
		{
			ActionManagerClient mngr_client;
			PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
			if (CastTo(mngr_client,player.GetActionManager()))
			{
				ActionTarget atrg = new ActionTarget(itemBase,null,-1,vector.Zero,-1.0);
		
				for (int i = 0; i < actions.Count(); i++)
				{
					ActionBase action = ActionBase.Cast(actions[i]);
					if (mngr_client.GetAction(action.Type()).Can(player,atrg,null))
					{
						cmenu.Add(action.GetText(), this, "CtxMenu_ExecAction", new Param2<EntityAI, ActionBase>(itemBase, action));
					}
				}
			}	
		}
		
		int actionMenuPosX, actionMenuPosY;
		GetMousePos(actionMenuPosX, actionMenuPosY);
		actionMenuPosX -= 5;
		actionMenuPosY -= 5;
		cmenu.Show(actionMenuPosX, actionMenuPosY);
	}

	void CtxMenu_ExecAction(EntityAI item, ActionBase action)
	{
		ActionManagerClient mngr_client;
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (CastTo(mngr_client,player.GetActionManager()))
		{
			ActionTarget atrg = new ActionTarget(item,null,-1,vector.Zero,-1.0);
			
			if (action.Can(player,atrg,null))
			{
				mngr_client.PerformActionStart(action,atrg,null);
			}
		}
	}
}
