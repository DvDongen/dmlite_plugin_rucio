/**
 * Copyright European Organization for Nuclear Research (CERN)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Authors:
 * - Mario Lassnig, <mario.lassnig@cern.ch>, 2012
 * - Daan van Dongen, <Daanvdongen@gmail.com>, 2013
 */

#ifndef RUCIO_CATALOG_H
#define RUCIO_CATALOG_H

#include <deque>
#include <string>

#include <dmlite/cpp/catalog.h>
#include <dmlite/cpp/dmlite.h>
#include <dmlite/cpp/dummy/DummyCatalog.h>
#include <dmlite/cpp/inode.h>

#include "rucio_connect.h"

namespace Rucio {

class RucioDID : public dmlite::Directory {
  public:
    explicit RucioDID(std::string path);

    ~RucioDID();
    std::string path;
    std::deque<std::string> scopes;
    std::deque<std::string> dids;
    std::deque<std::string> types;
    std::deque<std::string> RSE;
    long ptr;
    dmlite::ExtendedStat stat;
};

class RucioCatalog : public dmlite::DummyCatalog {
  public:
    RucioCatalog(dmlite::Catalog *next, std::string host, std::string port, std::string auth_token, std::string ca_cert) throw (dmlite::DmException);
    ~RucioCatalog();

    std::string getImplId() const throw ();

    void addReplica(const dmlite::Replica& replica) throw (dmlite::DmException);
    void changeDir(const std::string& path) throw (dmlite::DmException);
    void closeDir(dmlite::Directory *dir) throw (dmlite::DmException);
    void create(const std::string& path, mode_t mode) throw (dmlite::DmException);
    dmlite::ExtendedStat extendedStat(const std::string& path, bool followSym) throw (dmlite::DmException);
    std::string getComment(const std::string& path) throw (dmlite::DmException);
    dmlite::Replica getReplica(const std::string *rfn) throw (dmlite::DmException);

    std::vector<dmlite::Replica> getReplicas(const std::string& path) throw (dmlite::DmException);
    std::string getWorkingDir() throw (dmlite::DmException);
    void makeDir(const std::string& path, mode_t mode) throw (dmlite::DmException);
    dmlite::Directory *openDir(const std::string& path) throw (dmlite::DmException);
    struct dirent *readDir(dmlite::Directory *dir) throw (dmlite::DmException);
    dmlite::ExtendedStat *readDirx(dmlite::Directory *dir) throw (dmlite::DmException);
    std::string readLink(const std::string& path) throw (dmlite::DmException);
    void removeDir(const std::string& path) throw (dmlite::DmException);
    void rename(const std::string& oldPath, const std::string& newPath) throw (dmlite::DmException);
    void setAcl(const std::string& path, const dmlite::Acl& acl) throw (dmlite::DmException);
    void setChecksum(const std::string& path, const std::string& csumtype, const std::string& csumvalue) throw (dmlite::DmException);
    void setComment(const std::string& path, const std::string& comment) throw (dmlite::DmException);
    void setGuid(const std::string& path, const std::string& guid) throw (dmlite::DmException);
    void setMode(const std::string& path, mode_t mode) throw (dmlite::DmException);
    void setOwner(const std::string& path, uid_t newUid, gid_t newGid, bool followSymlink) throw (dmlite::DmException);
    void setSecurityContext(const dmlite::SecurityContext *ctx) throw (dmlite::DmException);
    void setSize(const std::string& path, size_t newSize) throw (dmlite::DmException);
    void setStackInstance(dmlite::StackInstance *si) throw (dmlite::DmException);
    void symlink(const std::string& path, const std::string symlink) throw (dmlite::DmException);
    mode_t umask(mode_t mask) throw ();
    void unlink(const std::string& path) throw (dmlite::DmException);
    void updateExtendedAttributes(const std::string& path, const dmlite::Extensible& attr) throw (dmlite::DmException);
    void updateReplica(const dmlite::Replica& replica) throw (dmlite::DmException);
    void utime(const std::string& path, const struct utimbuf *buf) throw (dmlite::DmException);

  private:
    std::deque<std::string> __splitPath(std::string path);
    std::string __sanitizePath(std::string path);
    std::string __seperatePath(std::string path);
    void __debugPrintPath();

    RucioConnect *rc;
    std::deque<std::string> cwd;
};
}

std::string UpToLow(std::string str);
std::string to_string(int number);

#endif
