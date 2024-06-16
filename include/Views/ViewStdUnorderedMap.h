#pragma once

#include "Configuration.h"
#include "QtHelpers/AbstractContainerView.h"
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{

    class ViewStdUnorderedMap : public AbstractContainerView
    {
        Q_OBJECT
      public:
        ViewStdUnorderedMap(uintptr_t address, const std::string& keytypeName, const std::string& valuetypeName, QWidget* parent = nullptr);

      protected:
        void reloadContainer() override;
      protected slots:
        void onItemCollapsed(const QModelIndex& index);

      private:
        MemoryField mKeyField;
        MemoryField mValueField;
        uintptr_t mMapAddress;
        uint8_t mMapAlignment;
    };
} // namespace S2Plugin
