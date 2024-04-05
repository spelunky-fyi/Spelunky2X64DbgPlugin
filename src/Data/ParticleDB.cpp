#include "Data/ParticleDB.h"
#include "Configuration.h"

size_t S2Plugin::ParticleDB::particleSize()
{
    // [Known Issue]: Static value, have to restart programm for size to update
    static size_t particleDBRecordSize = []()
    {
        auto& fields = Configuration::get()->typeFields(MemoryFieldType::ParticleDB);
        size_t size = 0;
        for (auto& field : fields)
        {
            size += field.get_size();
        }
        return size;
    }();
    return particleDBRecordSize;
}
