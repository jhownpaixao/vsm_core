/**
 * @brief Classe para manipulação de quantidade
 * @note Esta classe é usada para lidar com características específicas da quantidade de um item
 */
class VSMQuantityComponent extends VSMObjectComponent
{
    int m_Quantity;

    override void OnVirtualize(ItemBase virtualize) 
    {
        m_Quantity =  virtualize.GetQuantity();
        VSM_Debug("OnVirtualize", "Salvando quantidade de: " + virtualize.GetType() + " quantidade: " + m_Quantity.ToString());
    }

    override bool OnRestore(ItemBase restored) 
    {
        restored.SetQuantity(m_Quantity);
        VSM_Debug("OnRestore", "Restaurando quantidade de: " + restored.GetType() + " quantidade: " + m_Quantity.ToString());
        return true;
    }
}