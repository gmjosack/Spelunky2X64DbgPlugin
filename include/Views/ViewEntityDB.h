#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QStandardItemModel>
#include <QTableWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <vector>

namespace S2Plugin
{
    struct ViewToolbar;
    struct StyledItemDelegateHTML;
    struct TreeViewMemoryFields;
    struct MemoryField;

    class ViewEntityDB : public QWidget
    {
        Q_OBJECT
      public:
        ViewEntityDB(ViewToolbar* toolbar, size_t index = 1, QWidget* parent = nullptr);
        void showIndex(size_t index);

      protected:
        void closeEvent(QCloseEvent* event) override;
        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

      private slots:
        void searchFieldReturnPressed();
        void searchFieldCompleterActivated(const QString& text);
        void label();
        void fieldUpdated(const QString& fieldName);
        void fieldExpanded(const QModelIndex& index);
        void comparisonFieldChosen(const QString& fieldName);
        void compareGroupByCheckBoxClicked(int state);
        void comparisonCellClicked(int row, int column);
        void groupedComparisonItemClicked(QTreeWidgetItem* item, int column);

      private:
        ViewToolbar* mToolbar;
        std::unique_ptr<StyledItemDelegateHTML> mHTMLDelegate;

        QVBoxLayout* mMainLayout;
        QTabWidget* mMainTabWidget;
        QWidget* mTabLookup;
        QWidget* mTabCompare;

        // LOOKUP
        //size_t mLookupIndex;
        uintptr_t mEntityDBPtr;
        TreeViewMemoryFields* mMainTreeView;
        QLineEdit* mSearchLineEdit;
        QCompleter* mEntityNameCompleter;

        // COMPARE
        QComboBox* mCompareFieldComboBox;
        QTableWidget* mCompareTableWidget;
        QTreeWidget* mCompareTreeWidget;

        void initializeUI();
        void updateFieldValues();
        void populateComparisonTableWidget();
        void populateComparisonTreeWidget();
    };
} // namespace S2Plugin
