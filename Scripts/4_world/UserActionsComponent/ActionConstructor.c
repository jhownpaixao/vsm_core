modded class ActionConstructor 
{
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);
		
        actions.Insert(ActionVSM_Close);
        actions.Insert(ActionVSM_Open);
    }
};