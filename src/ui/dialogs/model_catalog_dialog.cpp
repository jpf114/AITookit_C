#include "ui/dialogs/model_catalog_dialog.h"

#include "core/catalog_url_validator.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

namespace aitoolkit::ui {

namespace {

QVector<CatalogModelEntry> parseCatalogDocument(const QJsonDocument& document) {
    if (!document.isObject()) {
        return {};
    }

    const QJsonArray models = document.object().value(QStringLiteral("models")).toArray();
    QVector<CatalogModelEntry> entries;
    entries.reserve(models.size());
    for (const QJsonValue& value : models) {
        const QJsonObject object = value.toObject();
        const QString downloadUrl = object.value(QStringLiteral("url")).toString();
        if (!core::validateModelDownloadUrlOrError(downloadUrl).isEmpty()) {
            continue;
        }

        entries.append(CatalogModelEntry{
            object.value(QStringLiteral("name")).toString(),
            object.value(QStringLiteral("taskType")).toString(),
            object.value(QStringLiteral("fileName")).toString(),
            downloadUrl,
            object.value(QStringLiteral("description")).toString(),
            object.value(QStringLiteral("inputSize")).toInt(640),
            object.value(QStringLiteral("decoder")).toString(),
            object.value(QStringLiteral("labelsCategory")).toString(),
        });
    }
    return entries;
}

QString defaultModelCatalogUrl() {
    return QStringLiteral(
        "https://raw.githubusercontent.com/jpf114/AITookit_C/main/resources/model_catalog.json");
}

QString resolvedCatalogUrl(const QString& catalogUrl) {
    return catalogUrl.trimmed().isEmpty() ? defaultModelCatalogUrl() : catalogUrl.trimmed();
}

QString catalogCachePath() {
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (cacheDir.isEmpty()) {
        return {};
    }
    return QDir(cacheDir).filePath(QStringLiteral("model_catalog_cache.json"));
}

void writeCatalogCache(const QByteArray& payload) {
    const QString cachePath = catalogCachePath();
    if (cachePath.isEmpty() || payload.isEmpty()) {
        return;
    }

    QDir().mkpath(QFileInfo(cachePath).absolutePath());
    QFile cacheFile(cachePath);
    if (cacheFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        cacheFile.write(payload);
    }
}

QVector<CatalogModelEntry> loadCatalogCache() {
    const QString cachePath = catalogCachePath();
    if (cachePath.isEmpty()) {
        return {};
    }

    QFile file(cachePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    return parseCatalogDocument(QJsonDocument::fromJson(file.readAll()));
}

QVector<CatalogModelEntry> loadCatalogFromJson() {
    const QDir appDir(QCoreApplication::applicationDirPath());
    const QStringList candidates = {
        appDir.filePath(QStringLiteral("share/model_catalog.json")),
        appDir.filePath(QStringLiteral("../share/model_catalog.json")),
        appDir.filePath(QStringLiteral("../../share/model_catalog.json")),
        QDir::current().filePath(QStringLiteral("resources/model_catalog.json")),
        QDir::current().filePath(QStringLiteral("../resources/model_catalog.json")),
    };

    for (const QString& path : candidates) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }

        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
        const QVector<CatalogModelEntry> entries = parseCatalogDocument(document);
        if (!entries.isEmpty()) {
            return entries;
        }
    }
    return {};
}

}  // namespace

ModelCatalogDialog::ModelCatalogDialog(
    const QString& modelsDir,
    const QString& catalogUrl,
    QWidget* parent)
    : QDialog(parent), modelsDir_(modelsDir), catalogUrl_(catalogUrl) {
    setWindowTitle(tr("模型目录"));
    setMinimumSize(600, 500);

    auto* layout = new QVBoxLayout(this);

    auto* titleLabel = new QLabel(tr("选择要下载的模型："), this);
    titleLabel->setObjectName(QStringLiteral("CatalogTitleLabel"));

    auto* licenseNotice = new QLabel(
        tr("注意：Ultralytics YOLO 模型采用 AGPL-3.0 许可证。"
           "商业使用需获取 Ultralytics 商业许可，详见 "
           "<a href=\"https://ultralytics.com/license\">ultralytics.com/license</a>"),
        this);
    licenseNotice->setWordWrap(true);
    licenseNotice->setObjectName(QStringLiteral("CatalogLicenseNotice"));
    licenseNotice->setOpenExternalLinks(true);

    auto* filterRow = new QHBoxLayout();
    auto* filterLabel = new QLabel(tr("任务类型："), this);
    filterCombo_ = new QComboBox(this);
    filterCombo_->addItem(tr("全部"), QString());
    filterCombo_->addItem(tr("目标检测"), QStringLiteral("detection"));
    filterCombo_->addItem(tr("图像分类"), QStringLiteral("classification"));
    filterCombo_->addItem(tr("实例分割"), QStringLiteral("segmentation"));
    filterRow->addWidget(filterLabel);
    filterRow->addWidget(filterCombo_);
    filterRow->addStretch(1);

    catalogList_ = new QListWidget(this);
    catalogList_->setSelectionMode(QAbstractItemView::SingleSelection);

    descriptionLabel_ = new QLabel(this);
    descriptionLabel_->setWordWrap(true);
    descriptionLabel_->setMinimumHeight(60);
    descriptionLabel_->setObjectName(QStringLiteral("CatalogDescriptionLabel"));

    downloadButton_ = new QPushButton(tr("下载"), this);
    downloadButton_->setObjectName(QStringLiteral("PrimaryButton"));
    downloadButton_->setAccessibleName(tr("下载模型"));
    downloadButton_->setEnabled(false);

    licenseAcceptCheckBox_ = new QCheckBox(
        tr("我已阅读并同意 Ultralytics YOLO 模型的 AGPL-3.0 许可条款"), this);
    licenseAcceptCheckBox_->setObjectName(QStringLiteral("CatalogLicenseAccept"));
    connect(licenseAcceptCheckBox_, &QCheckBox::toggled, this, [this](const bool checked) {
        updateDescription();
        if (!checked && downloadButton_->isEnabled()) {
            downloadButton_->setEnabled(false);
        }
    });

    auto* cancelButton = new QPushButton(tr("取消"), this);
    cancelButton->setObjectName(QStringLiteral("SecondaryButton"));
    cancelButton->setAccessibleName(tr("取消"));

    auto* buttonRow = new QHBoxLayout();
    buttonRow->addStretch(1);
    buttonRow->addWidget(cancelButton);
    buttonRow->addWidget(downloadButton_);

    layout->addWidget(titleLabel);
    layout->addWidget(licenseNotice);
    layout->addLayout(filterRow);
    layout->addWidget(catalogList_, 1);
    layout->addWidget(descriptionLabel_);
    layout->addWidget(licenseAcceptCheckBox_);
    layout->addLayout(buttonRow);

    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        applyFilter(filterCombo_->currentData().toString());
    });
    connect(catalogList_, &QListWidget::currentRowChanged, this, &ModelCatalogDialog::updateDescription);
    connect(catalogList_, &QListWidget::itemDoubleClicked, this, [this]() {
        if (downloadButton_->isEnabled()) accept();
    });
    connect(downloadButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    networkManager_ = new QNetworkAccessManager(this);
    remoteCatalogTimeout_ = new QTimer(this);
    remoteCatalogTimeout_->setSingleShot(true);

    populateCatalogFromLocal();
    startRemoteCatalogFetch();
}

ModelCatalogDialog::~ModelCatalogDialog() {
    if (remoteCatalogReply_ != nullptr) {
        remoteCatalogReply_->abort();
        remoteCatalogReply_->deleteLater();
        remoteCatalogReply_ = nullptr;
    }
}

void ModelCatalogDialog::populateCatalogFromLocal() {
    QVector<CatalogModelEntry> localEntries = loadCatalogFromJson();
    if (localEntries.isEmpty()) {
        localEntries = loadCatalogCache();
    }
  if (!localEntries.isEmpty()) {
        setCatalogEntries(localEntries);
    } else {
        descriptionLabel_->setText(tr("正在加载模型目录..."));
    }
}

void ModelCatalogDialog::startRemoteCatalogFetch() {
    const QString resolvedUrl = resolvedCatalogUrl(catalogUrl_);
    const QString urlError = core::validateCatalogUrlOrError(resolvedUrl);
    if (!urlError.isEmpty()) {
        if (entries_.isEmpty()) {
            descriptionLabel_->setText(tr("模型目录 URL 无效：%1").arg(urlError));
        }
        return;
    }

    const QUrl url(resolvedUrl);
    if (remoteCatalogReply_ != nullptr) {
        remoteCatalogReply_->abort();
        remoteCatalogReply_->deleteLater();
        remoteCatalogReply_ = nullptr;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("AIToolkit-ModelCatalog/1.0"));
    remoteCatalogReply_ = networkManager_->get(request);
    connect(remoteCatalogReply_, &QNetworkReply::finished, this, &ModelCatalogDialog::handleRemoteCatalogFinished);
    connect(remoteCatalogTimeout_, &QTimer::timeout, this, [this]() {
        if (remoteCatalogReply_ != nullptr) {
            remoteCatalogReply_->abort();
        }
    });
    remoteCatalogTimeout_->start(30000);
}

void ModelCatalogDialog::handleRemoteCatalogFinished() {
    remoteCatalogTimeout_->stop();
    if (remoteCatalogReply_ == nullptr) {
        return;
    }

    QNetworkReply* reply = remoteCatalogReply_;
    remoteCatalogReply_ = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        if (entries_.isEmpty()) {
            const QVector<CatalogModelEntry> cachedEntries = loadCatalogCache();
            if (!cachedEntries.isEmpty()) {
                setCatalogEntries(cachedEntries);
            } else {
                descriptionLabel_->setText(tr("无法加载模型目录，请检查网络连接。"));
            }
        }
        return;
    }

    const QByteArray payload = reply->readAll();
    reply->deleteLater();

    const QVector<CatalogModelEntry> remoteEntries = parseCatalogDocument(QJsonDocument::fromJson(payload));
    if (remoteEntries.isEmpty()) {
        if (entries_.isEmpty()) {
            descriptionLabel_->setText(tr("远程模型目录为空或无效。"));
        }
        return;
    }

    writeCatalogCache(payload);
    setCatalogEntries(remoteEntries);
}

void ModelCatalogDialog::setCatalogEntries(const QVector<CatalogModelEntry>& entries) {
    entries_ = entries;
    applyFilter(filterCombo_->currentData().toString());
}

void ModelCatalogDialog::applyFilter(const QString& taskType) {
    catalogList_->clear();
    for (int i = 0; i < entries_.size(); ++i) {
        const CatalogModelEntry& entry = entries_[i];
        if (!taskType.isEmpty() && entry.taskType != taskType) {
            continue;
        }
        const QString taskLabel = entry.taskType == QStringLiteral("segmentation")
            ? tr("分割")
            : entry.taskType == QStringLiteral("classification")
                ? tr("分类")
                : tr("检测");
        auto* item = new QListWidgetItem(
            QStringLiteral("%1 [%2]").arg(entry.name, taskLabel), catalogList_);
        item->setData(Qt::UserRole, i);
    }
    descriptionLabel_->clear();
    downloadButton_->setEnabled(false);
}

void ModelCatalogDialog::updateDescription() {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) {
        descriptionLabel_->clear();
        downloadButton_->setEnabled(false);
        return;
    }

    const int entryIndex = catalogList_->item(row)->data(Qt::UserRole).toInt();
    if (entryIndex < 0 || entryIndex >= entries_.size()) {
        descriptionLabel_->clear();
        downloadButton_->setEnabled(false);
        return;
    }

    const CatalogModelEntry& entry = entries_[entryIndex];
    descriptionLabel_->setText(entry.description);
    downloadButton_->setEnabled(licenseAcceptCheckBox_->isChecked());
}

QString ModelCatalogDialog::selectedModelName() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].name : QString();
}

QString ModelCatalogDialog::selectedModelUrl() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].url : QString();
}

QString ModelCatalogDialog::selectedModelFileName() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].fileName : QString();
}

QString ModelCatalogDialog::selectedModelDecoder() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].decoder : QString();
}

QString ModelCatalogDialog::selectedModelLabelsCategory() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].labelsCategory : QString();
}

QString ModelCatalogDialog::selectedModelTaskType() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return {};
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].taskType : QString();
}

int ModelCatalogDialog::selectedModelInputSize() const {
    const int row = catalogList_->currentRow();
    if (row < 0 || row >= catalogList_->count()) return 640;
    const int idx = catalogList_->item(row)->data(Qt::UserRole).toInt();
    return (idx >= 0 && idx < entries_.size()) ? entries_[idx].inputSize : 640;
}

QString ModelCatalogDialog::modelsDir() const {
    return modelsDir_;
}

}  // namespace aitoolkit::ui
