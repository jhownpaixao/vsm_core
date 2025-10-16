/**
 * @brief Classe para registro de manipuladores de componentes
 * @note Use para registrar os manipuladores que serão usados para lidar com os componentes de objetos
 */
class VSMObjectComponentConstructor extends VSMBase
{
    bool ConstructComponents(int version, out array<ref TObjComponent> componentsArray)
	{
		TTypenameArray component_array_names = new TTypenameArray;
		RegisterComponent(component_array_names, version);
		
		componentsArray = new array<ref TObjComponent>;
		TObjComponent new_component;

		for(int i = 0; i < component_array_names.Count(); i++)
		{
			new_component = TObjComponent.Cast(component_array_names[i].Spawn());
			if(new_component)
			{
				componentsArray.Insert(new_component);
			}
		}
		return true;
	}

    void RegisterComponent(TTypenameArray components, int version)
    {
        components.Insert(VSMHealthComponent);
        components.Insert(VSMAmmunitionComponent);
        components.Insert(VSMMagazineComponent);
        components.Insert(VSMWeaponComponent);
        components.Insert(VSMStoreComponent);
    }
}