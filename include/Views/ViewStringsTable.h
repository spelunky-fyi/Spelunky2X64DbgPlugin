#pragma once

#include <QLineEdit>
#include <QModelIndex>
#include <QSize>
#include <QString>
#include <QTableView>
#include <QWidget>
#include <cstdint>

namespace S2Plugin
{
    class SortFilterProxyModelStringsTable;

    constexpr uint8_t gsColStringID = 0;
    constexpr uint8_t gsColStringTableOffset = 1;
    constexpr uint8_t gsColStringMemoryOffset = 2;
    constexpr uint8_t gsColStringValue = 3;

    class ViewStringsTable : public QWidget
    {
        Q_OBJECT
      public:
        ViewStringsTable(QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      protected slots:
        void cellClicked(const QModelIndex& index);
        void filterTextChanged(const QString& text);
        void reload();

      private:
        QTableView* mMainTableView;
        SortFilterProxyModelStringsTable* mModelProxy;
    };

} // namespace S2Plugin
