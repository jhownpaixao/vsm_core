
modded class ActionDigInStash
{	
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if(super.ActionCondition(player, target, item))
		{
			ItemBase targetItem = ItemBase.Cast(target.GetObject());
			return !targetItem.VSM_IsProcessing();
		}
		return false;
	}
}
