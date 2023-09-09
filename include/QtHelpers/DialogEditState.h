#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

namespace S2Plugin
{
    enum class MemoryFieldType;
    struct ItemModelStates;
    struct Configuration;
    struct SortFilterProxyModelStates;

    class DialogEditState : public QDialog
    {
        Q_OBJECT

      public:
        DialogEditState(Configuration* config, const QString& fieldName, size_t memoryOffset, MemoryFieldType type, QWidget* parent = nullptr);

      protected:
        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

      private slots:
        void cancelBtnClicked();
        void changeBtnClicked();
        void stateComboBoxChanged(int index);

      private:
        size_t mMemoryOffset;
        MemoryFieldType mFieldType;

        QComboBox* mStatesComboBox;
        ItemModelStates* mStatesModel;
        SortFilterProxyModelStates* mStatesSortProxy;
        QLineEdit* mStateLineEdit;
    };
} // namespace S2Plugin
