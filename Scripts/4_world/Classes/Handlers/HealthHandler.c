class VSM_HealthHandler extends VirtualObjectHandler_Base
{
    float m_Health;
    ref array<ref DamageZoneProfile> m_DmgZones = {};

    override bool OnStoreLoad(ParamsReadContext ctx, int version) 
    {
        if (!ctx.Read(m_Health)) return false;
        if (!ctx.Read(m_DmgZones)) return false;

        return true;
    }

    override void OnStoreSave(ParamsWriteContext ctx) 
    {
        ctx.Write(m_Health);
        ctx.Write(m_DmgZones);
    }

    override void OnVirtualize(ItemBase virtualize, ItemBase parent) 
    {
        m_Health = virtualize.GetHealth();
        TStringArray dmgZones = new TStringArray();
        virtualize.GetDamageZones(dmgZones);

        foreach (string dmgZone: dmgZones)
		{
			float dmgZoneHealth = virtualize.GetHealth(dmgZone, "Health");
			DamageZoneProfile zoneProfile = new DamageZoneProfile();
            zoneProfile.name = dmgZone;
            zoneProfile.health = dmgZoneHealth;

            m_DmgZones.Insert(zoneProfile);
		}
    }

    override bool OnRestore(ItemBase restored, ItemBase parent) 
    {

        restored.SetHealth(m_Health);

        foreach (DamageZoneProfile zoneProfile: m_DmgZones)
		{
			restored.SetHealth(zoneProfile.name, "Health", zoneProfile.health);
		}
        return true;
    }
}

class DamageZoneProfile
{
    string name;
    float health;
}