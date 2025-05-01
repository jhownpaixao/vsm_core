class ActionVSM_Open: ActionInteractBase
{
	void ActionVSM_Open()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text = "#open";
	}

	override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
		Object target_object = target.GetObject();
		if ( target_object.IsItemBase() )
		{
			ItemBase container = ItemBase.Cast( target_object );
			if( container )
			{
				if(!container.VSM_IsOpen() && !container.VSM_IsProcessing() && !container.IsDamageDestroyed())
				{
					return true;
				}
			}
		}
		return false;
	}

	override void OnExecuteServer( ActionData action_data )
	{
		Object target_object = action_data.m_Target.GetObject();
		ItemBase container = ItemBase.Cast( target_object );
		if( container )
		{
			if (!container.VSM_CanOpen())
			{
				VirtualUtils.SendMessageToPlayer(action_data.m_Player, "STR_VSM_NOT_OPEN_CONTAINER");
				return;
			}
			container.VSM_Open();
		}
	}
};