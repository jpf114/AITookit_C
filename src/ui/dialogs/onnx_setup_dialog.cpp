#include "ui/dialogs/onnx_setup_dialog.h"

#include <QFileInfo>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

namespace aitoolkit::ui::dialogs {

OnnxSetupDialog::OnnxSetupDialog(const QString& onnxPath, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("配置 ONNX 模型"));
    setMinimumWidth(420);

    const QFileInfo info(onnxPath);
    const QString baseName = info.completeBaseName();

    nameEdit_ = new QLineEdit(baseName, this);
    widthSpin_ = new QSpinBox(this);
    widthSpin_->setRange(32, 4096);
    widthSpin_->setValue(640);

    heightSpin_ = new QSpinBox(this);
    heightSpin_->setRange(32, 4096);
    heightSpin_->setValue(640);

    confSpin_ = new QDoubleSpinBox(this);
    confSpin_->setRange(0.0, 1.0);
    confSpin_->setSingleStep(0.05);
    confSpin_->setDecimals(2);
    confSpin_->setValue(0.25);

    nmsSpin_ = new QDoubleSpinBox(this);
    nmsSpin_->setRange(0.0, 1.0);
    nmsSpin_->setSingleStep(0.05);
    nmsSpin_->setDecimals(2);
    nmsSpin_->setValue(0.45);

    labelsEdit_ = new QTextEdit(this);
    labelsEdit_->setMaximumHeight(120);
    labelsEdit_->setPlaceholderText(QStringLiteral("每行一个标签，可留空"));

    auto* formLayout = new QFormLayout;
    formLayout->addRow(QStringLiteral("模型名称："), nameEdit_);
    formLayout->addRow(QStringLiteral("输入宽度："), widthSpin_);
    formLayout->addRow(QStringLiteral("输入高度："), heightSpin_);
    formLayout->addRow(QStringLiteral("置信度阈值："), confSpin_);
    formLayout->addRow(QStringLiteral("NMS 阈值："), nmsSpin_);
    formLayout->addRow(QStringLiteral("标签列表："), labelsEdit_);

    auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    okButton_ = buttonBox->button(QDialogButtonBox::Ok);
    okButton_->setEnabled(false);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(nameEdit_, &QLineEdit::textChanged, this, &OnnxSetupDialog::updateOkButtonState);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(new QLabel(QStringLiteral("ONNX 文件：%1").arg(info.fileName()), this));
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    updateOkButtonState();
}

QString OnnxSetupDialog::modelName() const {
    return nameEdit_->text().trimmed();
}

int OnnxSetupDialog::inputWidth() const {
    return widthSpin_->value();
}

int OnnxSetupDialog::inputHeight() const {
    return heightSpin_->value();
}

double OnnxSetupDialog::confidenceThreshold() const {
    return confSpin_->value();
}

double OnnxSetupDialog::nmsThreshold() const {
    return nmsSpin_->value();
}

QStringList OnnxSetupDialog::labels() const {
    const QString text = labelsEdit_->toPlainText().trimmed();
    if (text.isEmpty()) {
        return {};
    }
    return text.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
}

void OnnxSetupDialog::updateOkButtonState() {
    okButton_->setEnabled(!nameEdit_->text().trimmed().isEmpty());
}

}  // namespace aitoolkit::ui::dialogs
