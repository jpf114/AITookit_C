#include "ui/license_utils.h"

#include <QMessageBox>

namespace aitoolkit::ui {

bool confirmAgplModelDownload(QWidget* parent) {
    const QMessageBox::StandardButton reply = QMessageBox::question(
        parent,
        QObject::tr("模型许可确认"),
        QObject::tr(
            "即将下载 Ultralytics YOLO 模型（AGPL-3.0 许可证）。\n\n"
            "• 个人学习与非商业用途通常可自由使用\n"
            "• 商业用途需获取 Ultralytics 商业许可\n"
            "• 详情：https://ultralytics.com/license\n\n"
            "是否继续下载？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    return reply == QMessageBox::Yes;
}

}  // namespace aitoolkit::ui
