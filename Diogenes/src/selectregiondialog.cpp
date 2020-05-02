#include "selectregiondialog.h"
#include "ui_selectregiondialog.h"


SelectRegionDialog::SelectRegionDialog(QWidget *parent, std::set<std::string> AvailableRegions) :
    QDialog(parent),
    ui(new Ui::SelectRegionDialog),
    AvailableRegions(AvailableRegions) {

    ui->setupUi(this);

    // Connect buttons
    connect(this->ui->ButtonBox, SIGNAL(accepted()), this, SLOT(OkButtonPressed()));
    connect(this->ui->ButtonBox, SIGNAL(rejected()), this, SLOT(CancelButtonPressed()));

    // Set up region combo box
    this->ui->RegionComboBox->addItem("Select Region");
    this->PopulateRegionComboBox();
}

void SelectRegionDialog::PopulateRegionComboBox() {
    for (auto region : this->AvailableRegions) {
        this->ui->RegionComboBox->addItem(QString(region.c_str()));
    }
}

void SelectRegionDialog::OkButtonPressed() {
    std::string selected_region = this->ui->RegionComboBox->currentText().toStdString().c_str();
    bool is_valid_region = this->AvailableRegions.find(selected_region) != this->AvailableRegions.end();
    if (is_valid_region) {
        this->SelectedRegion = selected_region;
        emit this->RegionIsSet();
        this->accept();
    }
}

void SelectRegionDialog::CancelButtonPressed() {
    this->accept();  // TODO: Exit program
}

SelectRegionDialog::~SelectRegionDialog() {
    delete ui;
}
