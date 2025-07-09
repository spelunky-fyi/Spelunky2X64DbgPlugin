#pragma once

#include "Configuration.h"
#include "QtHelpers/TreeViewMemoryFields.h"
#include "Spelunky2.h"
#include "ViewStruct.h"

namespace S2Plugin
{
    class ViewEntityFactory : public ViewStruct
    {
        Q_OBJECT
      public:
        explicit ViewEntityFactory(QWidget* parent = nullptr) : ViewStruct(0, {}, "EntityFactory", parent)
        {
            auto config = Configuration::get();
            MemoryField dummy;
            dummy.type = MemoryFieldType::Dummy;
            dummy.name = "EntityDB here";
            auto& entityDB = Spelunky2::get()->get_EntityDB();
            auto address = entityDB.addressOfIndex(0);
            mMainTreeView->addMemoryField(dummy, {}, address, 0);
            auto offset = entityDB.entitySize() * (config->entityList().highestID() + 2); // +2 for the id 0 and one extra slot at the end
            mMainTreeView->addMemoryFields(config->typeFields(MemoryFieldType::EntityFactory), "EntityFactory", address + offset, offset);
        }

      protected:
        QSize sizeHint() const override
        {
            return QSize(750, 200);
        }
    };
} // namespace S2Plugin
