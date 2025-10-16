//!DEPRECATED
modded class ActionTakeMaterialToHands
{
	override void OnExecuteServer( ActionData action_data )
	{
		super.OnExecuteServer(action_data);
		Object target_object = action_data.m_Target.GetObject();
		ItemBase container = ItemBase.Cast( target_object );
		if( container )
		{
			if (!container.VSM_IsVirtualStorage())
				return;

			VSMGameLabsIntegration gamelabs = new VSMGameLabsIntegration(action_data.m_Player, container);
			gamelabs.ReportContainerInteraction(VSM_ReportAction.TAKE);
		}
	}
}