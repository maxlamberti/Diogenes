#ifndef SELECTREGIONDIALOG_H
#define SELECTREGIONDIALOG_H

#include <QDialog>
#include "awsutils.hpp"

namespace Ui {
class SelectRegionDialog;
}

class SelectRegionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectRegionDialog(QWidget *parent = nullptr, std::set<std::string> AvailableRegions = {});
    ~SelectRegionDialog();
    std::set<std::string> AvailableRegions;
    std::string SelectedRegion;

private:
    Ui::SelectRegionDialog *ui;

private slots:
    void ConfirmButtonPressed();
    void PopulateRegionComboBox();

signals:
    void RegionIsSet();

};

#endif // SELECTREGIONDIALOG_H
