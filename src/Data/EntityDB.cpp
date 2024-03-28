#include "Data/EntityDB.h"
#include "Configuration.h"

size_t S2Plugin::EntityDB::entitySize()
{
    // [Known Issue]: Static value, have to restart programm for size to update
    static size_t entityDBRecordSize = []()
    {
        auto& fields = Configuration::get()->typeFields(MemoryFieldType::EntityDB);
        size_t size = 0;
        for (auto& field : fields)
        {
            size += field.get_size();
        }
        return size;
    }();

    return entityDBRecordSize;
}
