/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UTILS_FILEMGMT_LIBFS_INCLUDE_FS_ERROR_H
#define UTILS_FILEMGMT_LIBFS_INCLUDE_FS_ERROR_H

#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>

namespace OHOS::FileManagement::ModuleFileIO {
using namespace std;

#if (defined IOS_PLATFORM) || (defined WIN_PLATFORM)
constexpr int EBADR = 53;
constexpr int EBADFD = 77;
constexpr int ERESTART = 85;
#endif
#ifdef WIN_PLATFORM
constexpr int EDQUOT = 122;
#endif
constexpr int UNKNOWN_ERR = -1;
constexpr int NO_TASK_ERR = -2;
constexpr int CANCEL_ERR = -3;
constexpr int ERRNO_NOERR = 0;
constexpr int ECONNECTIONFAIL = 45;
constexpr int ECONNECTIONABORT = 46;
constexpr int STORAGE_SERVICE_SYS_CAP_TAG = 13600000;
constexpr int FILEIO_SYS_CAP_TAG = 13900000;
constexpr int USER_FILE_MANAGER_SYS_CAP_TAG = 14000000;
constexpr int USER_FILE_SERVICE_SYS_CAP_TAG = 14300000;
constexpr int DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG = 22400000;
constexpr int SOFTBUS_TRANS_FILE_PERMISSION_DENIED = -426114938;
constexpr int SOFTBUS_TRANS_FILE_DISK_QUOTA_EXCEEDED = -426114937;
constexpr int SOFTBUS_TRANS_FILE_NO_MEMORY = -426114936;
constexpr int SOFTBUS_TRANS_FILE_NETWORK_ERROR = -426114935;
constexpr int SOFTBUS_TRANS_FILE_NOT_FOUND = -426114934;
constexpr int SOFTBUS_TRANS_FILE_EXISTED = -426114933;
constexpr int DFS_CANCEL_SUCCESS = 204;
const std::string FILEIO_TAG_ERR_CODE = "code";
const std::string FILEIO_TAG_ERR_DATA = "data";

enum ErrCodeSuffixOfFileIO {
    E_PERM = 1,
    E_NOENT,
    E_SRCH,
    E_INTR,
    E_IO,
    E_NXIO,
    E_2BIG,
    E_BADF,
    E_CHILD,
    E_AGAIN,
    E_NOMEM,
    E_ACCES,
    E_FAULT,
    E_BUSY,
    E_EXIST,
    E_XDEV,
    E_NODEV,
    E_NOTDIR,
    E_ISDIR,
    E_INVAL,
    E_NFILE,
    E_MFILE,
    E_TXTBSY,
    E_FBIG,
    E_NOSPC,
    E_SPIPE,
    E_ROFS,
    E_MLINK,
    E_DEADLK,
    E_NAMETOOLONG,
    E_NOSYS,
    E_NOTEMPTY,
    E_LOOP,
    E_WOULDBLOCK,
    E_BADR,
    E_NOSTR,
    E_NODATA,
    E_OVERFLOW,
    E_BADFD,
    E_RESTART,
    E_DQUOT,
    E_UKERR,
    E_NOLCK,
    E_NETUNREACH,
    E_CONNECTION_FAIL,
    E_CONNECTION_ABORT,
    E_NOTASK,
    E_UNCANCELED,
    E_CANCELED,
};

enum ErrCodeSuffixOfUserFileManager {
    E_DISPLAYNAME = 1,
    E_URIM,
    E_SUFFIX,
    E_TRASH,
    E_OPEN_MODE,
    E_NOT_ALBUM,
    E_ROOT_DIR,
    E_MOVE_DENIED,
    E_RENAME_DENIED,
    E_RELATIVEPATH,
    E_INNER_FAIL,
    E_FILE_TYPE,
    E_FILE_KEY,
    E_INPUT
};

enum ErrCodeSuffixOfStorageService {
    E_IPCSS = 1,
    E_NOTSUPPORTEDFS,
    E_MOUNT,
    E_UNMOUNT,
    E_VOLUMESTATE,
    E_PREPARE,
    E_DELETE,
    E_NOOBJECT,
    E_OUTOFRANGE
};

enum ErrCodeSuffixOfUserFileService {
    E_IPCS = 1,
    E_URIS,
    E_GETINFO,
    E_GETRESULT,
    E_REGISTER,
    E_REMOVE,
    E_INIT,
    E_NOTIFY,
    E_CONNECT,
    E_CALLBACK_AND_URI_HAS_NOT_RELATIONS,
    E_CALLBACK_IS_NOT_REGISTER,
    E_CAN_NOT_FIND_URI,
    E_DO_NOT_HAVE_PARENT,
    E_LOAD_SA,
    E_COUNT
};

enum ErrCodeSuffixOfDistributedFile {
    E_CLOUD_NOT_READY = 1,
    E_NETWORK_ERR,
    E_BATTERY_WARNING,
    E_EXCEED_MAX_LIMIT,
    E_DATABASE_FAILED
};

enum CommonErrCode {
    E_PERMISSION = 201,
    E_PERMISSION_SYS = 202,
    E_PARAMS = 401,
    E_DEVICENOTSUPPORT = 801,
    E_OSNOTSUPPORT = 901,
    E_UNKNOWN_ERROR = 13900042
};

static inline std::unordered_map<int, int> softbusErr2ErrCodeTable {
    { SOFTBUS_TRANS_FILE_PERMISSION_DENIED, EPERM },
    { SOFTBUS_TRANS_FILE_DISK_QUOTA_EXCEEDED, EIO },
    { SOFTBUS_TRANS_FILE_NO_MEMORY, ENOMEM },
    { SOFTBUS_TRANS_FILE_NETWORK_ERROR, ENETUNREACH },
    { SOFTBUS_TRANS_FILE_NOT_FOUND, ENOENT },
    { SOFTBUS_TRANS_FILE_EXISTED, EEXIST },
    { DFS_CANCEL_SUCCESS, ECANCELED },
};

static inline std::unordered_map<std::string_view, int> uvCode2ErrCodeTable {
    { "EPERM", EPERM },
    { "ENOENT", ENOENT },
    { "ESRCH", ESRCH },
    { "EINTR", EINTR },
    { "EIO", EIO },
    { "ENXIO", ENXIO },
    { "E2BIG", E2BIG },
    { "EBADF", EBADF },
    { "ECHILD", ECHILD },
    { "EAGAIN", EAGAIN },
    { "ENOMEM", ENOMEM },
    { "EACCES", EACCES },
    { "EFAULT", EFAULT },
    { "EBUSY", EBUSY },
    { "EEXIST", EEXIST },
    { "EXDEV", EXDEV },
    { "ENODEV", ENODEV },
    { "ENOTDIR", ENOTDIR },
    { "EISDIR", EISDIR },
    { "EINVAL", EINVAL },
    { "ENFILE", ENFILE },
    { "EMFILE", EMFILE },
    { "ETXTBSY", ETXTBSY },
    { "EFBIG", EFBIG },
    { "ENOSPC", ENOSPC },
    { "ESPIPE", ESPIPE },
    { "EROFS", EROFS },
    { "EMLINK", EMLINK },
    { "EDEADLK", EDEADLK },
    { "ENAMETOOLONG", ENAMETOOLONG },
    { "ENOSYS", ENOSYS },
    { "ENOTEMPTY", ENOTEMPTY },
    { "ELOOP", ELOOP },
    { "EWOULDBLOCK", EWOULDBLOCK },
    { "EBADR", EBADR },
    { "ENOSTR", ENOSTR },
    { "ENODATA", ENODATA },
    { "EOVERFLOW", EOVERFLOW },
    { "EBADFD", EBADFD },
    { "ERESTART", ERESTART },
    { "EDQUOT", EDQUOT },
    { "ENETUNREACH", ENETUNREACH },
    { "ECONNECTIONFAIL", ECONNECTIONFAIL },
    { "ECONNECTIONABORT", ECONNECTIONABORT },
    { "ECANCELED", ECANCELED },
};

static inline std::unordered_map<int, std::pair<int32_t, std::string>> errCodeTable {
    { ERRNO_NOERR, { ERRNO_NOERR, "No error imformation" } },
    { EPERM, { FILEIO_SYS_CAP_TAG + E_PERM, "Operation not permitted" } },
    { ENOENT, { FILEIO_SYS_CAP_TAG + E_NOENT, "No such file or directory" } },
    { ESRCH, { FILEIO_SYS_CAP_TAG + E_SRCH, "No such process" } },
    { EINTR, { FILEIO_SYS_CAP_TAG + E_INTR, "Interrupted system call" } },
    { EIO, { FILEIO_SYS_CAP_TAG + E_IO, "I/O error" } },
    { ENXIO, { FILEIO_SYS_CAP_TAG + E_NXIO, "No such device or address" } },
    { E2BIG, { FILEIO_SYS_CAP_TAG + E_2BIG, "Arg list too long" } },
    { EBADF, { FILEIO_SYS_CAP_TAG + E_BADF, "Bad file descriptor" } },
    { ECHILD, { FILEIO_SYS_CAP_TAG + E_CHILD, "No child processes" } },
    { EAGAIN, { FILEIO_SYS_CAP_TAG + E_AGAIN, "Try again" } },
    { ENOMEM, { FILEIO_SYS_CAP_TAG + E_NOMEM, "Out of memory" } },
    { EACCES, { FILEIO_SYS_CAP_TAG + E_ACCES, "Permission denied" } },
    { EFAULT, { FILEIO_SYS_CAP_TAG + E_FAULT, "Bad address" } },
    { EBUSY, { FILEIO_SYS_CAP_TAG + E_BUSY, "Device or resource busy" } },
    { EEXIST, { FILEIO_SYS_CAP_TAG + E_EXIST, "File exists" } },
    { EXDEV, { FILEIO_SYS_CAP_TAG + E_XDEV, "Cross-device link" } },
    { ENODEV, { FILEIO_SYS_CAP_TAG + E_NODEV, "No such device" } },
    { ENOTDIR, { FILEIO_SYS_CAP_TAG + E_NOTDIR, "Not a directory" } },
    { EISDIR, { FILEIO_SYS_CAP_TAG + E_ISDIR, "Is a directory" } },
    { EINVAL, { FILEIO_SYS_CAP_TAG + E_INVAL, "Invalid argument" } },
    { ENFILE, { FILEIO_SYS_CAP_TAG + E_NFILE, "File table overflow" } },
    { EMFILE, { FILEIO_SYS_CAP_TAG + E_MFILE, "Too many open files" } },
    { ETXTBSY, { FILEIO_SYS_CAP_TAG + E_TXTBSY, "Text file busy" } },
    { EFBIG, { FILEIO_SYS_CAP_TAG + E_FBIG, "File too large" } },
    { ENOSPC, { FILEIO_SYS_CAP_TAG + E_NOSPC, "No space left on device" } },
    { ESPIPE, { FILEIO_SYS_CAP_TAG + E_SPIPE, "Illegal seek" } },
    { EROFS, { FILEIO_SYS_CAP_TAG + E_ROFS, "Read-only file system" } },
    { EMLINK, { FILEIO_SYS_CAP_TAG + E_MLINK, "Too many links" } },
    { EDEADLK, { FILEIO_SYS_CAP_TAG + E_DEADLK, "Resource deadlock would occur" } },
    { ENAMETOOLONG, { FILEIO_SYS_CAP_TAG + E_NAMETOOLONG, "File name too long" } },
    { ENOSYS, { FILEIO_SYS_CAP_TAG + E_NOSYS, "Function not implemented" } },
    { ENOTEMPTY, { FILEIO_SYS_CAP_TAG + E_NOTEMPTY, "Directory not empty" } },
    { ELOOP, { FILEIO_SYS_CAP_TAG + E_LOOP, "Too many symbolic links encountered" } },
    { EWOULDBLOCK, { FILEIO_SYS_CAP_TAG + E_WOULDBLOCK, "Operation would block" } },
    { EBADR, { FILEIO_SYS_CAP_TAG + E_BADR, "Invalid request descriptor" } },
    { ENOSTR, { FILEIO_SYS_CAP_TAG + E_NOSTR, "Device not a stream" } },
    { ENODATA, { FILEIO_SYS_CAP_TAG + E_NODATA, "No data available" } },
    { EOVERFLOW, { FILEIO_SYS_CAP_TAG + E_OVERFLOW, "Value too large for defined data type" } },
    { EBADFD, { FILEIO_SYS_CAP_TAG + E_BADFD, "File descriptor in bad state" } },
    { ERESTART, { FILEIO_SYS_CAP_TAG + E_RESTART, "Interrupted system call should be restarted" } },
    { EDQUOT, { FILEIO_SYS_CAP_TAG + E_DQUOT, "Quota exceeded" } },
    { UNKNOWN_ERR, { FILEIO_SYS_CAP_TAG + E_UKERR, "Unknown error" } },
    { ENOLCK, { FILEIO_SYS_CAP_TAG + E_NOLCK, "No record locks available" } },
    { ENETUNREACH, { FILEIO_SYS_CAP_TAG + E_NETUNREACH, "Network is unreachable" } },
    { ECONNECTIONFAIL, { FILEIO_SYS_CAP_TAG + E_CONNECTION_FAIL, "Connection failed" } },
    { ECONNECTIONABORT, { FILEIO_SYS_CAP_TAG + E_CONNECTION_ABORT, "Software caused connection abort" } },
    { NO_TASK_ERR, { FILEIO_SYS_CAP_TAG + E_NOTASK, "No task can be canceled" } },
    { CANCEL_ERR, { FILEIO_SYS_CAP_TAG + E_UNCANCELED, "Failed to cancel" } },
    { ECANCELED, { FILEIO_SYS_CAP_TAG + E_CANCELED, "Operation canceled" } },
    { FILEIO_SYS_CAP_TAG + E_PERM, { FILEIO_SYS_CAP_TAG + E_PERM, "Operation not permitted" } },
    { FILEIO_SYS_CAP_TAG + E_NOENT, { FILEIO_SYS_CAP_TAG + E_NOENT, "No such file or directory" } },
    { FILEIO_SYS_CAP_TAG + E_SRCH, { FILEIO_SYS_CAP_TAG + E_SRCH, "No such process" } },
    { FILEIO_SYS_CAP_TAG + E_INTR, { FILEIO_SYS_CAP_TAG + E_INTR, "Interrupted system call" } },
    { FILEIO_SYS_CAP_TAG + E_IO, { FILEIO_SYS_CAP_TAG + E_IO, "I/O error" } },
    { FILEIO_SYS_CAP_TAG + E_NXIO, { FILEIO_SYS_CAP_TAG + E_NXIO, "No such device or address" } },
    { FILEIO_SYS_CAP_TAG + E_2BIG, { FILEIO_SYS_CAP_TAG + E_2BIG, "Arg list too long" } },
    { FILEIO_SYS_CAP_TAG + E_BADF, { FILEIO_SYS_CAP_TAG + E_BADF, "Bad file descriptor" } },
    { FILEIO_SYS_CAP_TAG + E_CHILD, { FILEIO_SYS_CAP_TAG + E_CHILD, "No child processes" } },
    { FILEIO_SYS_CAP_TAG + E_AGAIN, { FILEIO_SYS_CAP_TAG + E_AGAIN, "Try again" } },
    { FILEIO_SYS_CAP_TAG + E_NOMEM, { FILEIO_SYS_CAP_TAG + E_NOMEM, "Out of memory" } },
    { FILEIO_SYS_CAP_TAG + E_ACCES, { FILEIO_SYS_CAP_TAG + E_ACCES, "Permission denied" } },
    { FILEIO_SYS_CAP_TAG + E_FAULT, { FILEIO_SYS_CAP_TAG + E_FAULT, "Bad address" } },
    { FILEIO_SYS_CAP_TAG + E_BUSY, { FILEIO_SYS_CAP_TAG + E_BUSY, "Device or resource busy" } },
    { FILEIO_SYS_CAP_TAG + E_EXIST, { FILEIO_SYS_CAP_TAG + E_EXIST, "File exists" } },
    { FILEIO_SYS_CAP_TAG + E_XDEV, { FILEIO_SYS_CAP_TAG + E_XDEV, "Cross-device link" } },
    { FILEIO_SYS_CAP_TAG + E_NODEV, { FILEIO_SYS_CAP_TAG + E_NODEV, "No such device" } },
    { FILEIO_SYS_CAP_TAG + E_NOTDIR, { FILEIO_SYS_CAP_TAG + E_NOTDIR, "Not a directory" } },
    { FILEIO_SYS_CAP_TAG + E_ISDIR, { FILEIO_SYS_CAP_TAG + E_ISDIR, "Is a directory" } },
    { FILEIO_SYS_CAP_TAG + E_INVAL, { FILEIO_SYS_CAP_TAG + E_INVAL, "Invalid argument" } },
    { FILEIO_SYS_CAP_TAG + E_NFILE, { FILEIO_SYS_CAP_TAG + E_NFILE, "File table overflow" } },
    { FILEIO_SYS_CAP_TAG + E_MFILE, { FILEIO_SYS_CAP_TAG + E_MFILE, "Too many open files" } },
    { FILEIO_SYS_CAP_TAG + E_TXTBSY, { FILEIO_SYS_CAP_TAG + E_TXTBSY, "Text file busy" } },
    { FILEIO_SYS_CAP_TAG + E_FBIG, { FILEIO_SYS_CAP_TAG + E_FBIG, "File too large" } },
    { FILEIO_SYS_CAP_TAG + E_NOSPC, { FILEIO_SYS_CAP_TAG + E_NOSPC, "No space left on device" } },
    { FILEIO_SYS_CAP_TAG + E_SPIPE, { FILEIO_SYS_CAP_TAG + E_SPIPE, "Illegal seek" } },
    { FILEIO_SYS_CAP_TAG + E_ROFS, { FILEIO_SYS_CAP_TAG + E_ROFS, "Read-only file system" } },
    { FILEIO_SYS_CAP_TAG + E_MLINK, { FILEIO_SYS_CAP_TAG + E_MLINK, "Too many links" } },
    { FILEIO_SYS_CAP_TAG + E_DEADLK, { FILEIO_SYS_CAP_TAG + E_DEADLK, "Resource deadlock would occur" } },
    { FILEIO_SYS_CAP_TAG + E_NAMETOOLONG, { FILEIO_SYS_CAP_TAG + E_NAMETOOLONG, "File name too long" } },
    { FILEIO_SYS_CAP_TAG + E_NOSYS, { FILEIO_SYS_CAP_TAG + E_NOSYS, "Function not implemented" } },
    { FILEIO_SYS_CAP_TAG + E_NOTEMPTY, { FILEIO_SYS_CAP_TAG + E_NOTEMPTY, "Directory not empty" } },
    { FILEIO_SYS_CAP_TAG + E_LOOP, { FILEIO_SYS_CAP_TAG + E_LOOP, "Too many symbolic links encountered" } },
    { FILEIO_SYS_CAP_TAG + E_WOULDBLOCK, { FILEIO_SYS_CAP_TAG + E_WOULDBLOCK, "Operation would block" } },
    { FILEIO_SYS_CAP_TAG + E_BADR, { FILEIO_SYS_CAP_TAG + E_BADR, "Invalid request descriptor" } },
    { FILEIO_SYS_CAP_TAG + E_NOSTR, { FILEIO_SYS_CAP_TAG + E_NOSTR, "Device not a stream" } },
    { FILEIO_SYS_CAP_TAG + E_NODATA, { FILEIO_SYS_CAP_TAG + E_NODATA, "No data available" } },
    { FILEIO_SYS_CAP_TAG + E_OVERFLOW, { FILEIO_SYS_CAP_TAG + E_OVERFLOW, "Value too large for defined data type" } },
    { FILEIO_SYS_CAP_TAG + E_BADFD, { FILEIO_SYS_CAP_TAG + E_BADFD, "File descriptor in bad state" } },
    { FILEIO_SYS_CAP_TAG + E_RESTART,
        { FILEIO_SYS_CAP_TAG + E_RESTART, "Interrupted system call should be restarted" } },
    { FILEIO_SYS_CAP_TAG + E_DQUOT, { FILEIO_SYS_CAP_TAG + E_DQUOT, "Quota exceeded" } },
    { FILEIO_SYS_CAP_TAG + E_UKERR, { FILEIO_SYS_CAP_TAG + E_UKERR, "Unknown error" } },
    { FILEIO_SYS_CAP_TAG + E_NOLCK, { FILEIO_SYS_CAP_TAG + E_NOLCK, "No record locks available" } },
    { FILEIO_SYS_CAP_TAG + E_NETUNREACH, { FILEIO_SYS_CAP_TAG + E_NETUNREACH, "Network is unreachable" } },
    { FILEIO_SYS_CAP_TAG + E_CONNECTION_FAIL, { FILEIO_SYS_CAP_TAG + E_CONNECTION_FAIL, "Connection failed" } },
    { FILEIO_SYS_CAP_TAG + E_CONNECTION_ABORT,
        { FILEIO_SYS_CAP_TAG + E_CONNECTION_ABORT, "Software caused connection abort" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_DISPLAYNAME,
        { USER_FILE_MANAGER_SYS_CAP_TAG + E_DISPLAYNAME, "Invalid display name" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_URIM, { USER_FILE_MANAGER_SYS_CAP_TAG + E_URIM, "Invalid uri" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_SUFFIX,
        { USER_FILE_MANAGER_SYS_CAP_TAG + E_SUFFIX, "Invalid file extension" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_TRASH,
        { USER_FILE_MANAGER_SYS_CAP_TAG + E_TRASH, "File has been put into trash bin" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_OPEN_MODE,
        { USER_FILE_MANAGER_SYS_CAP_TAG + E_OPEN_MODE, "Invalid open mode" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_NOT_ALBUM,
        { USER_FILE_MANAGER_SYS_CAP_TAG + E_NOT_ALBUM, "The uri is not album" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_ROOT_DIR, { USER_FILE_MANAGER_SYS_CAP_TAG + E_ROOT_DIR, "Invalid root dir" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_MOVE_DENIED,
        { USER_FILE_MANAGER_SYS_CAP_TAG + E_MOVE_DENIED, "Failed to move as dir check failed" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_RENAME_DENIED,
        { USER_FILE_MANAGER_SYS_CAP_TAG + E_RENAME_DENIED, "Failed to rename as dir check failed" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_RELATIVEPATH,
        { USER_FILE_MANAGER_SYS_CAP_TAG + E_RELATIVEPATH, "Relative path not exist or invalid" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_INNER_FAIL,
        { USER_FILE_MANAGER_SYS_CAP_TAG + E_INNER_FAIL, "MediaLibrary inner fail" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_FILE_TYPE,
        { USER_FILE_MANAGER_SYS_CAP_TAG + E_FILE_TYPE, "File type is not allow in the directory" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_FILE_KEY, { USER_FILE_MANAGER_SYS_CAP_TAG + E_FILE_KEY, "Member not exist" } },
    { USER_FILE_MANAGER_SYS_CAP_TAG + E_INPUT, { USER_FILE_MANAGER_SYS_CAP_TAG + E_INPUT, "Wrong input parameter" } },
    { STORAGE_SERVICE_SYS_CAP_TAG + E_IPCSS, { STORAGE_SERVICE_SYS_CAP_TAG + E_IPCSS, "IPC error" } },
    { STORAGE_SERVICE_SYS_CAP_TAG + E_NOTSUPPORTEDFS,
        { STORAGE_SERVICE_SYS_CAP_TAG + E_NOTSUPPORTEDFS, "Not supported filesystem" } },
    { STORAGE_SERVICE_SYS_CAP_TAG + E_MOUNT, { STORAGE_SERVICE_SYS_CAP_TAG + E_MOUNT, "Failed to mount" } },
    { STORAGE_SERVICE_SYS_CAP_TAG + E_UNMOUNT, { STORAGE_SERVICE_SYS_CAP_TAG + E_UNMOUNT, "Failed to unmount" } },
    { STORAGE_SERVICE_SYS_CAP_TAG + E_VOLUMESTATE,
        { STORAGE_SERVICE_SYS_CAP_TAG + E_VOLUMESTATE, "Incorrect volume state" } },
    { STORAGE_SERVICE_SYS_CAP_TAG + E_PREPARE,
        { STORAGE_SERVICE_SYS_CAP_TAG + E_PREPARE, "Prepare directory or node error" } },
    { STORAGE_SERVICE_SYS_CAP_TAG + E_DELETE,
        { STORAGE_SERVICE_SYS_CAP_TAG + E_DELETE, "Delete directory or node error" } },
    { STORAGE_SERVICE_SYS_CAP_TAG + E_NOOBJECT, { STORAGE_SERVICE_SYS_CAP_TAG + E_NOOBJECT, "No such object" } },
    { STORAGE_SERVICE_SYS_CAP_TAG + E_OUTOFRANGE,
        { STORAGE_SERVICE_SYS_CAP_TAG + E_OUTOFRANGE, "User id out of range" } },
    { STORAGE_SERVICE_SYS_CAP_TAG + E_NOOBJECT, { STORAGE_SERVICE_SYS_CAP_TAG + E_NOOBJECT, "No such object" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_IPCS, { USER_FILE_SERVICE_SYS_CAP_TAG + E_IPCS, "IPC error" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_URIS, { USER_FILE_SERVICE_SYS_CAP_TAG + E_URIS, "Invalid uri" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_GETINFO,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_GETINFO, "Fail to get fileextension info" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_GETRESULT,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_GETRESULT, "Get wrong result" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_REGISTER,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_REGISTER, "Fail to register notification" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_REMOVE,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_REMOVE, "Fail to remove notification" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_INIT,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_INIT, "Fail to init notification agent" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_NOTIFY, { USER_FILE_SERVICE_SYS_CAP_TAG + E_NOTIFY, "Fail to notify agent" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_CONNECT,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_CONNECT, "Fail to connect file access extension ability" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_CALLBACK_AND_URI_HAS_NOT_RELATIONS,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_CALLBACK_AND_URI_HAS_NOT_RELATIONS,
            "The uri has no relationship with the callback and cannot be unregistered" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_CALLBACK_IS_NOT_REGISTER,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_CALLBACK_IS_NOT_REGISTER,
            "Cannot unregister the callback that has not been registered" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_CAN_NOT_FIND_URI,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_CAN_NOT_FIND_URI, "Can not find registered uri" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_DO_NOT_HAVE_PARENT,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_DO_NOT_HAVE_PARENT, "Do not have parent uri" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_LOAD_SA,
        { USER_FILE_SERVICE_SYS_CAP_TAG + E_LOAD_SA, "Fail to load system ability" } },
    { USER_FILE_SERVICE_SYS_CAP_TAG + E_COUNT, { USER_FILE_SERVICE_SYS_CAP_TAG + E_COUNT, "Too many records" } },
    { E_PERMISSION, { E_PERMISSION, "Permission verification failed" } },
    { E_PERMISSION_SYS, { E_PERMISSION_SYS, "The caller is not a system application" } },
    { E_PARAMS, { E_PARAMS, "The input parameter is invalid" } },
    { E_DEVICENOTSUPPORT, { E_DEVICENOTSUPPORT, "The device doesn't support this api" } },
    { E_OSNOTSUPPORT, { E_OSNOTSUPPORT, "The os doesn't support this api" } },
    { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_CLOUD_NOT_READY,
        { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_CLOUD_NOT_READY, "Cloud status not ready" } },
    { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_NETWORK_ERR,
        { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_NETWORK_ERR, "Network unavailable" } },
    { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_BATTERY_WARNING,
        { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_BATTERY_WARNING, "Battery level warning" } },
    { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_EXCEED_MAX_LIMIT,
        { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_EXCEED_MAX_LIMIT, "Exceed the maximum limit" } },
    { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_DATABASE_FAILED,
        { DISTRIBUTEDFILE_SERVICE_SYS_CAP_TAG + E_DATABASE_FAILED, "Database operation failed" } },
};

class FsError {
public:
    FsError(int errCode);
    int GetErrNo() const;
    const std::string &GetErrMsg() const;
    ~FsError() = default;
    explicit operator bool() const;

private:
    int errno_ = ERRNO_NOERR;
    std::string errMsg_;
};

} // namespace OHOS::FileManagement::ModuleFileIO
#endif // UTILS_FILEMGMT_LIBFS_INCLUDE_FS_ERROR_H
