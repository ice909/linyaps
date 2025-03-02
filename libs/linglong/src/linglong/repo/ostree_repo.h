/*
 * SPDX-FileCopyrightText: 2022-2024 UnionTech Software Technology Co., Ltd.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "linglong/api/types/v1/RepoConfigV2.hpp"
#include "linglong/package/fuzzy_reference.h"
#include "linglong/package/layer_dir.h"
#include "linglong/package/reference.h"
#include "linglong/package_manager/package_task.h"
#include "linglong/repo/client_factory.h"
#include "linglong/repo/repo_cache.h"
#include "linglong/utils/error/error.h"

#include <ostree.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace linglong::repo {

struct clearReferenceOption
{
    bool forceRemote = false;
    bool fallbackToRemote = true;
    bool queryOriginRepo = false;
};

class OSTreeRepo : public QObject
{
    Q_OBJECT
public:
    OSTreeRepo(const OSTreeRepo &) = delete;
    OSTreeRepo(OSTreeRepo &&) = delete;
    OSTreeRepo &operator=(const OSTreeRepo &) = delete;
    OSTreeRepo &operator=(OSTreeRepo &&) = delete;
    OSTreeRepo(const QDir &path,
               api::types::v1::RepoConfigV2 cfg,
               ClientFactory &clientFactory) noexcept;

    ~OSTreeRepo() override;

    [[nodiscard]] const api::types::v1::RepoConfigV2 &getConfig() const noexcept;
    utils::error::Result<void> setConfig(const api::types::v1::RepoConfigV2 &cfg) noexcept;

    utils::error::Result<package::LayerDir>
    importLayerDir(const package::LayerDir &dir,
                   std::vector<std::filesystem::path> overlays = {},
                   const std::optional<std::string> &subRef = std::nullopt) noexcept;

    [[nodiscard]] utils::error::Result<package::LayerDir>
    getLayerDir(const package::Reference &ref,
                const std::string &module = "binary",
                const std::optional<std::string> &subRef = std::nullopt) const noexcept;
    [[nodiscard]] utils::error::Result<void>
    push(const package::Reference &reference, const std::string &module = "binary") const noexcept;

    [[nodiscard]] utils::error::Result<void>
    pushToRemote(const std::string &remoteRepo,
                 const std::string &url,
                 const package::Reference &reference,
                 const std::string &module = "binary") const noexcept;

    void pull(service::PackageTask &taskContext,
              const package::Reference &reference,
              const std::string &module = "binary") noexcept;

    [[nodiscard]] utils::error::Result<package::Reference>
    clearReference(const package::FuzzyReference &fuzz,
                   const clearReferenceOption &opts,
                   const std::string &module = "binary") const noexcept;

    utils::error::Result<std::vector<api::types::v1::PackageInfoV2>> listLocal() const noexcept;
    utils::error::Result<std::vector<api::types::v1::PackageInfoV2>>
    listLocalLatest() const noexcept;
    utils::error::Result<std::vector<api::types::v1::PackageInfoV2>>
    listRemote(const package::FuzzyReference &fuzzyRef,
               const linglong::api::types::v1::Repo &repo = {}) const noexcept;
    [[nodiscard]] utils::error::Result<std::vector<api::types::v1::RepositoryCacheLayersItem>>
    listLocalBy(const linglong::repo::repoCacheQuery &query) const noexcept;

    utils::error::Result<void>
    remove(const package::Reference &ref,
           const std::string &module = "binary",
           const std::optional<std::string> &subRef = std::nullopt) noexcept;

    utils::error::Result<void> prune();

    void removeDanglingXDGIntergation() noexcept;
    // exportReference should be called when LayerDir of ref is existed in local repo
    void exportReference(const package::Reference &ref) noexcept;
    // unexportReference should be called when LayerDir of ref is existed in local repo
    void unexportReference(const package::Reference &ref) noexcept;
    void updateSharedInfo() noexcept;
    utils::error::Result<void>
    markDeleted(const package::Reference &ref,
                bool deleted,
                const std::string &module = "binary",
                const std::optional<std::string> &subRef = std::nullopt) noexcept;

    // 扫描layers变动，重新合并变动layer的modules
    [[nodiscard]] utils::error::Result<void> mergeModules() const noexcept;
    // 获取合并后的layerDir，如果没有找到则返回binary模块的layerDir
    [[nodiscard]] utils::error::Result<package::LayerDir>
    getMergedModuleDir(const package::Reference &ref, bool fallbackLayerDir = true) const noexcept;
    // 将指定的modules合并到临时目录，并返回合并后的layerDir，供打包者调试应用
    // 临时目录会在智能指针释放时删除
    [[nodiscard]] utils::error::Result<std::shared_ptr<package::LayerDir>>
    getMergedModuleDir(const package::Reference &ref, const QStringList &modules) const noexcept;
    std::vector<std::string> getModuleList(const package::Reference &ref) noexcept;
    [[nodiscard]] utils::error::Result<std::vector<std::string>>
    getRemoteModuleList(const package::Reference &ref,
                        const std::optional<std::vector<std::string>> &filter) const noexcept;

    [[nodiscard]] utils::error::Result<api::types::v1::RepositoryCacheLayersItem>
    getLayerItem(const package::Reference &ref,
                 std::string module = "binary",
                 const std::optional<std::string> &subRef = std::nullopt) const noexcept;
    utils::error::Result<void> fixExportAllEntries() noexcept;

private:
    api::types::v1::RepoConfigV2 cfg;

    struct OstreeRepoDeleter
    {
        void operator()(OstreeRepo *repo)
        {
            qDebug() << "delete OstreeRepo" << repo;
            g_clear_object(&repo);
        }
    };

    std::unique_ptr<OstreeRepo, OstreeRepoDeleter> ostreeRepo = nullptr;
    QDir repoDir;
    std::unique_ptr<linglong::repo::RepoCache> cache{ nullptr };
    ClientFactory &m_clientFactory;

    utils::error::Result<void> updateConfig(const api::types::v1::RepoConfigV2 &newCfg) noexcept;
    QDir ostreeRepoDir() const noexcept;
    [[nodiscard]] utils::error::Result<QDir>
    ensureEmptyLayerDir(const std::string &commit) const noexcept;
    utils::error::Result<void> handleRepositoryUpdate(
      QDir layerDir, const api::types::v1::RepositoryCacheLayersItem &layer) noexcept;
    utils::error::Result<void>
    removeOstreeRef(const api::types::v1::RepositoryCacheLayersItem &layer) noexcept;
    [[nodiscard]] utils::error::Result<package::LayerDir>
    getLayerDir(const api::types::v1::RepositoryCacheLayersItem &layer) const noexcept;

    // 获取合并后的layerDir，如果没有找到则返回binary模块的layerDir
    [[nodiscard]] utils::error::Result<package::LayerDir>
    getMergedModuleDir(const api::types::v1::RepositoryCacheLayersItem &layer,
                       bool fallbackLayerDir = true) const noexcept;
    utils::error::Result<void>
    exportEntries(const std::filesystem::path &rootEntriesDir,
                  const api::types::v1::RepositoryCacheLayersItem &item) noexcept;
    static utils::error::Result<void> IniLikeFileRewrite(const QFileInfo &info,
                                                         const QString &id) noexcept;

    utils::error::Result<void> exportDir(const std::string &appID,
                                         const std::filesystem::path &source,
                                         const std::filesystem::path &destination,
                                         const int &max_depth);
    // exportEntries will clear the entries/share and export all applications to the entries/share
    utils::error::Result<void> exportAllEntries() noexcept;
};

} // namespace linglong::repo
