#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QModelIndex>
#include <QSize>
#include <QString>
#include <QTabWidget>
#include <QTableView>
#include <QTableWidget>
#include <QWidget>

namespace S2Plugin
{
    class ItemModelVirtualTable;
    class SortFilterProxyModelVirtualTable;
    class ItemModelGatherVirtualData;
    class SortFilterProxyModelGatherVirtualData;

    class ViewVirtualTable : public QWidget
    {
        Q_OBJECT
      public:
        ViewVirtualTable(QWidget* parent = nullptr);
        void showLookupAddress(size_t address);
        void updateGatherProgress();

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void tableEntryClicked(const QModelIndex& index);
        void detectEntities();
        void showImportedSymbolsCheckBoxStateChanged(int state);
        void showNonAddressEntriesCheckBoxStateChanged(int state);
        void showSymbollessEntriesCheckBoxStateChanged(int state);
        void filterTextChanged(const QString& text);
        void processLookupAddressText();
        void gatherEntities();
        void gatherExtraObjects();
        void gatherAvailableVirtuals();
        void exportGatheredData();
        void exportVirtTable();
        void exportCppEnum();
        void showGatherHideCompletedCheckBoxStateChanged(int state);

      private:
        QTabWidget* mMainTabWidget;

        // DATA
        QTableView* mDataTable;
        ItemModelVirtualTable* mModel;
        SortFilterProxyModelVirtualTable* mSortFilterProxy;

        // LOOKUP
        QLineEdit* mLookupAddressLineEdit;
        QTableWidget* mLookupResultsTable;

        // GATHER
        ItemModelGatherVirtualData* mGatherModel;
        SortFilterProxyModelGatherVirtualData* mGatherSortFilterProxy;
        QLabel* mGatherProgressLabel;

        void initializeUI();
        void lookupAddress(size_t address);
    };
} // namespace S2Plugin
