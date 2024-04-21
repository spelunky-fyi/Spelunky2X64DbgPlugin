#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTextEdit>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>
#include <memory>

namespace S2Plugin
{
    class TreeViewMemoryFields;
    class WidgetMemoryView;
    class CPPSyntaxHighlighter;
    class WidgetSpelunkyLevel;

    class ViewEntity : public QWidget
    {
        Q_OBJECT
      public:
        ViewEntity(size_t entityOffset, QWidget* parent = nullptr);

      protected:
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void refreshEntity();
        void toggleAutoRefresh(int newState);
        void autoRefreshIntervalChanged(const QString& text);
        void interpretAsChanged(const QString& text);
        void label();
        void entityOffsetDropped(size_t entityOffset);
        void tabChanged();

      private:
        QVBoxLayout* mMainLayout;
        QHBoxLayout* mTopLayout;
        QTabWidget* mMainTabWidget;
        QWidget* mTabFields;
        QWidget* mTabMemory;
        QWidget* mTabLevel;
        QWidget* mTabCPP;

        uintptr_t mEntityPtr;
        uintptr_t mComparisonEntityPtr{0};
        size_t mEntitySize{0};

        // TOP LAYOUT
        QPushButton* mRefreshButton;
        QCheckBox* mAutoRefreshCheckBox;
        QLineEdit* mAutoRefreshIntervalLineEdit;
        std::unique_ptr<QTimer> mAutoRefreshTimer;
        QComboBox* mInterpretAsComboBox;

        // TAB FIELDS
        TreeViewMemoryFields* mMainTreeView;

        // TAB MEMORY
        WidgetMemoryView* mMemoryView;
        WidgetMemoryView* mMemoryComparisonView;
        QScrollArea* mMemoryComparisonScrollArea;

        // TAB LEVEL
        WidgetSpelunkyLevel* mSpelunkyLevel;

        // TAB CPP
        QTextEdit* mCPPTextEdit;
        CPPSyntaxHighlighter* mCPPSyntaxHighlighter;

        void initializeUI();
        void updateMemoryViewOffsetAndSize();
        void updateComparedMemoryViewHighlights();
    };
} // namespace S2Plugin
