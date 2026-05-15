#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QPushButton>

namespace aitoolkit::ui::dialogs {

class OnnxSetupDialog : public QDialog {
    Q_OBJECT

public:
    explicit OnnxSetupDialog(const QString& onnxPath, QWidget* parent = nullptr);

    QString modelName() const;
    int inputWidth() const;
    int inputHeight() const;
    double confidenceThreshold() const;
    double nmsThreshold() const;
    QStringList labels() const;

private:
    void updateOkButtonState();
    void tryAutoDetectInputSize(const QString& onnxPath);

    QLineEdit* nameEdit_ = nullptr;
    QSpinBox* widthSpin_ = nullptr;
    QSpinBox* heightSpin_ = nullptr;
    QDoubleSpinBox* confSpin_ = nullptr;
    QDoubleSpinBox* nmsSpin_ = nullptr;
    QTextEdit* labelsEdit_ = nullptr;
    QPushButton* okButton_ = nullptr;
};

}  // namespace aitoolkit::ui::dialogs
