#ifndef CREDENTIALSDIALOG_H
#define CREDENTIALSDIALOG_H

#include <QDialog>
#include "awsutils.hpp"


namespace Ui {
class CredentialsDialog;
}

class CredentialsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CredentialsDialog(QWidget *parent = nullptr, std::set<std::string> AvailableRegions = {});
    ~CredentialsDialog();
    std::string AccessKeyId;
    std::string SecretKey;
    std::string SelectedRegion;
    std::set<std::string> AvailableRegions;

private:
    Ui::CredentialsDialog *ui;

private slots:
    void OkButtonPressed();
    void CancelButtonPressed();
    void PopulateRegionComboBox();

signals:
    void DataIsSet();

};

#endif // CREDENTIALSDIALOG_H
