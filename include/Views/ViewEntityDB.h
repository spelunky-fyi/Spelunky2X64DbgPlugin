#pragma once

#include "QtHelpers/WidgetDatabaseView.h"

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewEntityDB : public WidgetDatabaseView
    {
        Q_OBJECT
      public:
        ViewEntityDB(QWidget* parent = nullptr);
        void showID(ID_type id) override;

      protected:
        QSize sizeHint() const override;
        void label() const override;
        ID_type highestRecordID() const override;
        bool isValidRecordID(ID_type id) const override;
        std::optional<ID_type> getIDForName(QString name) const override;
        QString recordNameForID(ID_type id) const override;
        uintptr_t addressOfRecordID(ID_type id) const override;

      private slots:
        void searchFieldCompleterActivated();
    };
} // namespace S2Plugin
