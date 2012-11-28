/**
 * Copyright European Organization for Nuclear Research (CERN)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Authors:
 * - Mario Lassnig, <mario.lassnig@cern.ch>, 2012
 */

#include <deque>
#include <iostream>
#include <string>

#include <dmlite/cpp/catalog.h>
#include <dmlite/cpp/dmlite.h>
#include <dmlite/cpp/inode.h>

#include "rucio_catalog.h"
#include "rucio_connect.h"

namespace Rucio {

RucioDID::RucioDID(std::string path) : dmlite::Directory() {
  std::cerr << "[RUCIO][DID][CTOR]" << std::endl;
  this->path = path;
  ptr = 0;
}

RucioDID::~RucioDID() {
  std::cerr << "[RUCIO][DID][DTOR]" << std::endl;
}

RucioCatalog::RucioCatalog(dmlite::Catalog *next, std::string host, std::string auth_token) throw (dmlite::DmException) :
  dmlite::DummyCatalog(next) {
  std::cerr << "[RUCIO][CATALOG][CTOR] " << next->getImplId() << std::endl;

  cwd = "/";

  rc = new RucioConnect(host, auth_token);
}

RucioCatalog::~RucioCatalog() {
  std::cerr << "[RUCIO][CATALOG][DTOR]" << std::endl;
  delete rc;
}

std::string RucioCatalog::getImplId() const throw () {
  return "plugin_rucio/0.1";
}

void RucioCatalog::addReplica(const dmlite::Replica& replica) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][ADDREPLICA]" << std::endl;
}

void RucioCatalog::changeDir(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][CHANGEDIR]" << std::endl;
  cwd = path;
}

void RucioCatalog::closeDir(dmlite::Directory *dir) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][CLOSEDIR]" << std::endl;
  if (dir != NULL) {
    delete dir;
  }
}

void RucioCatalog::create(const std::string& path, mode_t mode) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][CREATE]" << std::endl;
}

dmlite::ExtendedStat RucioCatalog::extendedStat(const std::string& path, bool followSym) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][EXTENDEDSTAT]" << std::endl;
  dmlite::ExtendedStat e_stat;
  e_stat.parent = (ino_t)0;
  // e_stat.stat = 0;
  // e_stat.status = FileStatus.kOnline;
  e_stat.name = "filename";
  e_stat.guid = "a82b801a-1cf5-11e2-96f7-003048f164d6";
  e_stat.csumtype = "adler32";
  e_stat.csumvalue = "11e60398";
  return e_stat;
}

std::string RucioCatalog::getComment(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETCOMMENT]" << std::endl;
  return "dummy_comment";
}

dmlite::Replica RucioCatalog::getReplica(const std::string *rfn) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETREPLICA]" << std::endl;
  dmlite::Replica dummy;
  return dummy;
}

std::vector<dmlite::Replica> RucioCatalog::getReplicas(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETREPLICAS]" << std::endl;
  std::vector<dmlite::Replica> dummy;
  return dummy;
}

std::string RucioCatalog::getWorkingDir() throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETWORKINGDIR]" << std::endl;
  return cwd;
}

void RucioCatalog::makeDir(const std::string& path, mode_t mode) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][MAKEDIR]" << std::endl;
}

dmlite::Directory *RucioCatalog::openDir(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][OPENDIR]" << std::endl;

  /**
   * TODO: Check if directory actually exists
   *
   * Needs additional peek server call. For now, just accept and hope for the best.
   */
  RucioDID *did_r = new RucioDID(path);

  /**
   * The root directory is special, because it's only a list of scopes
   */
  if (path == "/") {
    did_r->scopes = rc->list_scopes();
  } else {

    /**
     * Everything else is a combination of scope, DID, or both. So split it up and let's see what we've got.
     */
    std::deque<std::string> tokens;
    std::string::size_type tokenOff = 0, sepOff = tokenOff;
    while (sepOff != std::string::npos) {
      sepOff = path.find('/', sepOff);
      std::string::size_type tokenLen = (sepOff == std::string::npos) ? sepOff : sepOff++ - tokenOff;
      std::string token = path.substr(tokenOff, tokenLen);
      if (!token.empty()) {
        tokens.push_back(token);
      }
      tokenOff = sepOff;
    }

    /**
     * Just look at the last two entries.
     */
    std::string scope;
    std::string did;
    if (tokens.size() % 2) {
      scope = tokens.at(tokens.size() - 1);
    } else {
      did = tokens.at(tokens.size() - 1);
      scope = tokens.at(tokens.size() - 2);
    }
    std::deque<did_t> tmp_r = rc->list_dids(scope, did);
    for (uint i = 0; i < tmp_r.size(); ++i) {
      did_r->scopes.push_back(tmp_r.at(i).scope);
      did_r->dids.push_back(tmp_r.at(i).did);
      did_r->types.push_back(tmp_r.at(i).type);
    }
  }

  return did_r;
}

struct dirent *RucioCatalog::readDir(dmlite::Directory *dir) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][READDIR]" << std::endl;

  return NULL;
}

dmlite::ExtendedStat *RucioCatalog::readDirx(dmlite::Directory *dir) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][READDIRX]" << std::endl;

  RucioDID *did_r = dynamic_cast<RucioDID *> (dir);

  if ((did_r->ptr == did_r->scopes.size()) || (did_r->scopes.size() == 0)) {
    return NULL;
  }

  if (did_r->path == "/") {
    did_r->stat.name = "/" + did_r->scopes.at(did_r->ptr);
  } else {
    did_r->stat.name = "/" + did_r->scopes.at(did_r->ptr) + "/" + did_r->dids.at(did_r->ptr);
  }

  ++did_r->ptr;
  return &did_r->stat;
}

std::string RucioCatalog::readLink(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][READLINK]" << std::endl;
  return "dummy_link";
}

void RucioCatalog::removeDir(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][REMOVEDIR]" << std::endl;
}

void RucioCatalog::rename(const std::string& oldPath, const std::string& newPath) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][RENAME]" << std::endl;
}

void RucioCatalog::setAcl(const std::string& path, const dmlite::Acl& acl) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETACL]" << std::endl;
}

void RucioCatalog::setChecksum(const std::string& path, const std::string& csumtype, const std::string &
                               csumvalue) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETCHECKSUM]" << std::endl;
}

void RucioCatalog::setComment(const std::string& path, const std::string& comment) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETCOMMENT]" << std::endl;
}

void RucioCatalog::setGuid(const std::string& path, const std::string& guid) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETGUID]" << std::endl;
}

void RucioCatalog::setMode(const std::string& path, mode_t mode) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETMODE]" << std::endl;
}

void RucioCatalog::setOwner(const std::string& path, uid_t newUid, gid_t newGid, bool followSymlink) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETOWNER]" << std::endl;
}

void RucioCatalog::setSecurityContext(const dmlite::SecurityContext *ctx) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETSECURITYCONTEXT]" << std::endl;
}

void RucioCatalog::setSize(const std::string& path, size_t newSize) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETSIZE]" << std::endl;
}

void RucioCatalog::setStackInstance(dmlite::StackInstance *si) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETSTACKINSTANCE]" << std::endl;
}

void RucioCatalog::symlink(const std::string& path, const std::string symlink) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SYMLINK]" << std::endl;
}

mode_t RucioCatalog::umask(mode_t mask) throw () {
  std::cerr << "[RUCIO][CATALOG][UMASK]" << std::endl;
  return 0;
}

void RucioCatalog::unlink(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UNLINK]" << std::endl;
}

void RucioCatalog::updateExtendedAttributes(const std::string& path, const dmlite::Extensible& attr) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UPDATEEXTENDEDATTRIBUTES]" << std::endl;
}

void RucioCatalog::updateReplica(const dmlite::Replica& replica) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UPDATEREPLICA]" << std::endl;
}

void RucioCatalog::utime(const std::string& path, const struct utimbuf *buf) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UTIME]" << std::endl;
}
}
