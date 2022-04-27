/*
 * Copyright (c) 2021. Uniontech Software Ltd. All rights reserved.
 *
 * Author:     liujianqiang <liujianqiang@uniontech.com>
 *
 * Maintainer: liujianqiang <liujianqiang@uniontech.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

namespace linglong {
namespace dbus {
enum class RetCode {
    DataDirNotExists, //数据目录不存在
    UapJsonFileNotExists, // uap.json不存在
    BundleFileNotExists, // uab文件不存在
    user_input_param_err = 600,
    pkg_already_installed,
    pkg_not_installed,
    install_runtime_failed,
    load_pkg_data_failed,
    pkg_install_success, // 安装成功
    pkg_install_failed, // 安装失败
    pkg_uninstall_success, // 卸载成功
    pkg_uninstall_failed, // 卸载失败
    ErrorPkgUpdateFailed,
    ErrorPkgUpdateSuccess,
    ErrorPkgKillFailed,
    ErrorPkgKillSuccess, // kill成功
    ErrorPkgQuerySuccess, // 查询成功
    ErrorPkgQueryFailed // 查询失败
};

template<typename T = int>
inline T RetCode(RetCode code)
{
    return static_cast<T>(code);
}
} // namespace dbus

namespace Status {
enum StatusCode { SUCCESS = 0, FAIL = 1 };
}
} // namespace linglong
