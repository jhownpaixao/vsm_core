/** 
 * @brief Classe de fila para o processo de drop de itens
 * @note Esta classe é usada para gerenciar o processo de drop de itens, quando um storage é destruido no mundo
 */
class VSMLegacyDropQueue : VSMLegacyRestorationQueue
{
    override bool OnInit()
    {
        if(super.OnInit())
        {
            m_ForceSpawnOnGround = true;
            return true;
        }
        
        return false;
    }

    override void OnStart()
    {
        super.OnStart();
        VSM_Info("OnStart", m_Container.GetType() + " iniciando processo de drop de itens items: " + m_ItemCount);
    }

    override void OnComplete()
    {
        super.OnComplete();
        VirtualStorageModule.GetModule().OnDeleteContainer(m_Container);
        VSM_Info("OnComplete", m_Container.GetType() + " drop de itens concluido");
    }
}