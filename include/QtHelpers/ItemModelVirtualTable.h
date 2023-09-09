#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

namespace S2Plugin
{
    struct ViewToolbar;
    struct VirtualTableLookup;

    static const uint8_t gsColTableOffset = 0;
    static const uint8_t gsColCodeAddress = 1;
    static const uint8_t gsColTableAddress = 2;
    static const uint8_t gsColSymbolName = 3;

    class ItemModelVirtualTable : public QAbstractItemModel
    {
        Q_OBJECT

      public:
        ItemModelVirtualTable(VirtualTableLookup* vtl, QObject* parent = nullptr);

        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        void detectEntities(ViewToolbar* toolbar);

      private:
        VirtualTableLookup* mVirtualTableLookup;
    };

    class SortFilterProxyModelVirtualTable : public QSortFilterProxyModel
    {
        Q_OBJECT

      public:
        SortFilterProxyModelVirtualTable(VirtualTableLookup* vtl, QObject* parent = nullptr);

        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
        void setShowImportedSymbols(bool b);
        void setShowNonAddressEntries(bool b);
        void setShowSymbollessEntries(bool b);
        void setFilterString(const QString& f);

        bool symbollessEntriesShown() const noexcept;

      private:
        VirtualTableLookup* mVirtualTableLookup;
        bool mShowImportedSymbols = true;
        bool mShowNonAddressEntries = true;
        bool mShowSymbollessEntries = true;
        QString mFilterString = "";
    };

} // namespace S2Plugin
