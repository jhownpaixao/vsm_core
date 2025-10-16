/**
 * @brief Classe para manipulação de saúde de objetos
 * @note Esta classe é usada para lidar com características específicas de saúde e zonas
 */
class VSMHealthComponent extends VSMObjectComponent
{
    float m_Health;
    ref array<ref DamageZoneProfile> m_DmgZones = {};

    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    {
        if(version < VSM_StorageVersion.V_0510)
        {
            if (!ctx.Read(m_Health)) return false;
            if (!ctx.Read(m_DmgZones)) return false;
        }
        
        return true;
    }

    override void OnStoreSave(ParamsWriteContext ctx) 
    {
        // ctx.Write(m_Health);
        // ctx.Write(m_DmgZones);
    }

    override void OnVirtualize(ItemBase virtualize) 
    {
        m_Health = virtualize.GetHealth();
        
        TStringArray dmgZones = new TStringArray();
        virtualize.GetDamageZones(dmgZones);

        for (int i = 0; i < dmgZones.Count(); i++)
		{
			string dmgZone = dmgZones[i];
            if(!dmgZone) continue;

			float dmgZoneHealth = virtualize.GetHealth(dmgZone, "Health");
			DamageZoneProfile zoneProfile = new DamageZoneProfile();
            zoneProfile.name = dmgZone;
            zoneProfile.health = dmgZoneHealth;

            m_DmgZones.Insert(zoneProfile);
		}

        VSM_Debug("OnVirtualize", "Virtualizando componente de saúde para: " + virtualize.GetType() + ", health: " + m_Health.ToString() + ", zonas: " + m_DmgZones.Count().ToString());
    }

    override bool OnRestore(ItemBase restored) 
    {
        restored.SetHealth(m_Health);

        for (int i = 0; i < m_DmgZones.Count(); i++)
		{
			DamageZoneProfile zoneProfile = m_DmgZones[i];
            if(!zoneProfile) continue;

			restored.SetHealth(zoneProfile.name, "Health", zoneProfile.health);
		}

        VSM_Debug("OnRestore", "Restaurando componente de saúde para: " + restored.GetType() + ", health: " + m_Health.ToString() + ", zonas: " + m_DmgZones.Count().ToString());
        return true;
    }
}

class DamageZoneProfile
{
    string name;
    float health;
}