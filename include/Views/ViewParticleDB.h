#pragma once

#include "QtHelpers/StyledItemDelegateHTML.h"
#include <QCloseEvent>
#include <QComboBox>
#include <QLineEdit>
#include <QModelIndex>
#include <QSize>
#include <QStandardItem>
#include <QString>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>
#include <cstdint>

namespace S2Plugin
{
    class TreeViewMemoryFields;

    class ViewParticleDB : public QWidget
    {
        Q_OBJECT
      public:
        ViewParticleDB(uint32_t id = 1, QWidget* parent = nullptr);
        void showID(uint32_t id);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void searchFieldReturnPressed();
        void searchFieldCompleterActivated();
        void label();
        void fieldUpdated(int row, QStandardItem* parrent);
        void fieldExpanded(const QModelIndex& index);
        void comparisonFieldChosen();
        void compareGroupByCheckBoxClicked(int state);
        void comparisonCellClicked(int row, int column);
        void groupedComparisonItemClicked(QTreeWidgetItem* item);

      private:
        StyledItemDelegateHTML mHTMLDelegate;

        QTabWidget* mMainTabWidget;
        QWidget* mTabLookup;
        QWidget* mTabCompare;

        // LOOKUP
        TreeViewMemoryFields* mMainTreeView;
        QLineEdit* mSearchLineEdit;

        // COMPARE
        QComboBox* mCompareFieldComboBox;
        QTableWidget* mCompareTableWidget;
        QTreeWidget* mCompareTreeWidget;

        void initializeUI();
        void populateComparisonTableWidget();
        void populateComparisonTreeWidget();
    };
} // namespace S2Plugin
