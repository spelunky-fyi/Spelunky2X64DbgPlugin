#pragma once

#include "Configuration.h"
#include "QtHelpers/AbstractContainerView.h"
#include <QWidget>
#include <cstdint>
#include <string>

namespace S2Plugin
{
    class ViewStdList : public AbstractContainerView
    {
        Q_OBJECT
      public:
        ViewStdList(uintptr_t address, const std::string& valueTypeName, bool oldType = false, QWidget* parent = nullptr);

      protected:
        void reloadContainer() override;

      private:
        MemoryField mValueField;
        uintptr_t mListAddress;
        bool mOldType;
    };
} // namespace S2Plugin
