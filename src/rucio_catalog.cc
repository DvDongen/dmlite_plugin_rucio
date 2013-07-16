/**
 * Copyright European Organization for Nuclear Research (CERN)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Authors:
 * - Mario Lassnig, <mario.lassnig@cern.ch>, 2012
 * - Daan van Dongen, <Daanvandongen@gmail.com>, 2013
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
  //std::cerr << path << std::endl;

  this->path = path;
  ptr = 0;

  stat.guid = "0";
  stat.csumtype = "ad";
  stat.csumvalue = "00000000";
  stat.stat.st_mode = S_IFDIR;
  stat.stat.st_uid = 0;
  stat.stat.st_gid = 0;
  stat.stat.st_size = 0;
  stat.stat.st_atime = 0;
  stat.stat.st_mtime = 0;
  stat.stat.st_ctime = 0;
}

RucioDID::~RucioDID() {
  std::cerr << "[RUCIO][DID][DTOR]" << std::endl;
}

RucioCatalog::RucioCatalog(dmlite::Catalog *next, std::string host, std::string port, std::string auth_token,
                           std::string ca_cert) throw (dmlite::DmException) :
  dmlite::DummyCatalog(next) {
  std::cerr << "[RUCIO][CATALOG][CTOR] " << next->getImplId() << std::endl;
  rc = new RucioConnect(host, port , auth_token, ca_cert);
  //std::cerr << "https://" <<  host << ":" << port << std::endl;
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

  std::deque<std::string> original = cwd;

  std::string tmp_path = __sanitizePath(path);

  //std::cerr << tmp_path << std::endl;

  if (tmp_path == "~") { // Home is root
    if (!cwd.empty()) { // So remove everything else
      cwd.clear();
    }
  } else if (tmp_path == ".") { // Nothing to do
  } else if (tmp_path == "..") { // Go back up one
    if (!cwd.empty()) { // not necessary, if we're already at root
      cwd.erase(cwd.end());
    }
  } else { // Change to the given directory
    if (tmp_path == "/") { // Go back to root
      cwd.clear();
    } else if ((tmp_path.at(0) == '/') && (tmp_path.size() > 1)) { // Is it a full tmp_path?
      cwd = __splitPath(tmp_path);
    } else { // Or relative?
      if (tmp_path.find("/") != std::string::npos) { // Is it a hierarchy?
        if (cwd.empty()) { // Are we at root already?
          cwd.push_back(tmp_path); // If yes, just add the relative
        } else {
          cwd.push_back(tmp_path); // And the tmp_path
        }
      } else {
        std::deque<std::string> tmp_split = __splitPath(path);
        uint i = 0;
        for (uint i = 0; i < tmp_split.size(); ++i) {
          cwd.push_back(tmp_split.at(i));
        }
      }
    }
  }

  std::string tmp_scope;
  std::string tmp_did;



  if (cwd.empty()) {
    return;
  } else if (cwd.size() == 1) {
    bool found = false;
    std::deque<std::string> tmp_scopes = rc->list_scopes();
    for (uint i = 0; i < tmp_scopes.size(); ++i) {
      if (tmp_scopes.at(i) == cwd.back()) {
        found = true;
        break;
      }
    }
    if (!found) {
      //cwd = original;
	std::deque<std::string> tmp_sl;
	tmp_sl.push_back("///");				//Changing the name to something that doesn't exist, now OpenDir gives no data
	cwd = tmp_sl;
	std::cerr << "ERROR: Directory does not exist" <<std::endl;
      throw dmlite::DmException(DMLITE_SYSERR(1), "directory does not exist");
    }
  } else if (cwd.size() > 1) {
	bool isrses = FALSE;
	if (cwd.back() == "rses"){
		cwd.pop_back();
		isrses = TRUE;
	}

	if (cwd.back().find(":") != cwd.back().npos ){
		tmp_scope = cwd.back().substr(0, cwd.back().find(":"));
		tmp_did = cwd.back().substr(cwd.back().find(":") + 1);
	}
	else {
		tmp_scope = cwd.back();
		tmp_did = "";
	}

	did_t exists;
	exists = rc->get_did(tmp_scope, tmp_did);

    if (exists.scope.empty() && exists.name.empty() && exists.type.empty()) {
      cwd = original;
      throw dmlite::DmException(DMLITE_SYSERR(1), "directory does not exist");
    }

	if (isrses == TRUE){
	    cwd.back() = cwd.back() + "/rses";
	}
  }
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
  dmlite::ExtendedStat stat_e;

  stat_e.guid = "0";
  stat_e.csumtype = "ad";
  stat_e.csumvalue = "00000000";
  stat_e.stat.st_mode = S_IFDIR;
  stat_e.stat.st_uid = 0;
  stat_e.stat.st_gid = 0;
  stat_e.stat.st_size = 0;
  stat_e.stat.st_atime = 0;
  stat_e.stat.st_mtime = 0;
  stat_e.stat.st_ctime = 0;

  std::string tmp_path = __sanitizePath(path);

  if (tmp_path == "/") {
    stat_e.name = "/";
  } else {
    std::deque<std::string> splits = __splitPath(tmp_path);
    stat_e.name = splits.back();
    // stat_e.stat.st_mode = S_IFREG;
  }

  return stat_e;
}

std::string RucioCatalog::getComment(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETCOMMENT]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
  return std::string();
}

dmlite::Replica RucioCatalog::getReplica(const std::string *rfn) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETREPLICA]" << std::endl;
  dmlite::Replica dummy;
  return dummy;
}

std::vector<dmlite::Replica> RucioCatalog::getReplicas(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETREPLICAS]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
  std::vector<dmlite::Replica> dummy;
  return dummy;
}

std::string RucioCatalog::getWorkingDir() throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][GETWORKINGDIR]" << std::endl;
  std::string cwd_s = "/";
  for (int i = 0; i < cwd.size(); ++i) {
    cwd_s += cwd.at(i) + "/";
  }

  if (cwd_s != "/") {
    cwd_s = cwd_s.substr(0, cwd_s.size() - 1);
  }
  return cwd_s;
}

void RucioCatalog::makeDir(const std::string& path, mode_t mode) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][MAKEDIR]" << std::endl;
}

dmlite::Directory *RucioCatalog::openDir(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][OPENDIR]" << std::endl;

  std::string tmp_path = __sanitizePath(path);
  //std::cerr << tmp_path << std::endl;

  RucioDID *did_r = new RucioDID(tmp_path);

  did_r->scopes.push_back(".");
  did_r->scopes.push_back("..");
  did_r->dids.push_back(std::string());
  did_r->dids.push_back(std::string());
  did_r->types.push_back(std::string());
  did_r->types.push_back(std::string());
  did_r->RSE.push_back(std::string());
  did_r->RSE.push_back(std::string());

  /**
   * The root directory is special, because it's only a list of scopes
   */

  if (tmp_path == "/") {
    std::deque<std::string> tmp_scopes = rc->list_scopes();
    for (uint i = 0; i < tmp_scopes.size(); ++i) {
      did_r->scopes.push_back(tmp_scopes.at(i));
	did_r->dids.push_back(std::string());
	did_r->types.push_back("SCOPE");
	did_r->RSE.push_back(std::string());
    }
  } else {
    /**
     * Everything else is a combination of scope, DID, or both. So split it up and let's see what we've got.
     */
    std::deque<std::string> tokens;		//tokens = your path
    std::string::size_type tokenOff = 0, sepOff = tokenOff, tmp_sit = 0;
    while (sepOff != std::string::npos) {
      tmp_sit = tmp_path.find('/', sepOff);
      sepOff = tmp_sit;
      std::string::size_type tokenLen = (sepOff == std::string::npos) ? sepOff : sepOff++ - tokenOff;
      std::string token = tmp_path.substr(tokenOff, tokenLen);
      if (!token.empty()) {
        tokens.push_back(token);
      }
      tokenOff = sepOff;
    }

    /**
     * Just look at the last two entries.
     */

    bool isrses = FALSE;
    if (tokens.back() == "rses"){
	tokens.pop_back();
	isrses = TRUE;
    }

    if( cwd.size() > 1 && isrses == FALSE){				//adding RSEs
	did_r->scopes.push_back("rses");
	did_r->dids.push_back(std::string());
	did_r->types.push_back(std::string());
	did_r->RSE.push_back(std::string());
    }

    std::string tmp_scope = tokens.back().substr(0, tokens.back().find(":"));
    std::string tmp_did = tokens.back().substr(tokens.back().find(":") + 1);

    if ((tmp_scope == ".") && (tmp_did == ".")) {
      return did_r;
    }

    std::deque<did_t> tmp_r;
	if( isrses == TRUE){
		if (cwd.size() == 1 || cwd.size()==0) {
			tmp_r = rc->list_rses(tmp_scope, std::string());
		} else {
			tmp_r = rc->list_rses(tmp_scope, tmp_did);
		}
	}
	else {
		if (cwd.size() == 1 || cwd.size()==0) {
			tmp_r = rc->list_dids(tmp_scope, std::string());
		} else {
			tmp_r = rc->list_dids(tmp_scope, tmp_did);
		}
	}
    for (uint i = 0; i < tmp_r.size(); ++i) {
      // std::cerr << tmp_r.at(i).scope << tmp_r.at(i).name << tmp_r.at(i).type << std::endl;
      did_r->scopes.push_back(tmp_r.at(i).scope );
      did_r->dids.push_back(tmp_r.at(i).name);
      did_r->types.push_back(tmp_r.at(i).type);
      did_r->RSE.push_back(tmp_r.at(i).RSE);
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

  if ((did_r->ptr == did_r->scopes.size()) || (did_r->scopes.empty())) {
    return NULL;
  }

  //std::cerr << did_r->scopes.at(did_r->ptr) << std::endl;

  if (did_r->path == "/") {
    did_r->stat.name = did_r->scopes.at(did_r->ptr);
  } else {
    if (did_r->scopes.at(did_r->ptr).empty()) {
      did_r->stat.name = did_r->dids.at(did_r->ptr);
    } else {
      if (did_r->dids.at(did_r->ptr).empty()) {
        did_r->stat.name = did_r->scopes.at(did_r->ptr);
      } else {
	if (did_r->types.at(did_r->ptr).empty() ){
		std::cerr << "ERROR: " << did_r->dids.at(did_r->ptr) << " DID without type" <<std::endl;
	} else {
	  if (did_r->types.at(did_r->ptr) == "CONTAINER" || did_r->types.at(did_r->ptr) == "DATASET" ){
		did_r->stat.name = did_r->scopes.at(did_r->ptr) ;
	  } else {
	    if (did_r->types.at(did_r->ptr) == "FILE" ){
		did_r->stat.name = did_r->scopes.at(did_r->ptr);
		did_r->stat.stat.st_mode = S_IFREG;
	    } else {
	      if(did_r->types.at(did_r->ptr) == "RSE") {
		did_r->stat.name = did_r->scopes.at(did_r->ptr);
              } else {
		std::cerr << "ERROR: Unknown DID-type" << std::endl;
		did_r->stat.name = did_r->scopes.at(did_r->ptr);
	      }
	    }
	  }
        }
      }
    }
  }

  ++did_r->ptr;
  return &did_r->stat;
}

std::string RucioCatalog::readLink(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][READLINK]" << std::endl;
  std::string tmp_path = __sanitizePath(path);

  return std::string();
}

void RucioCatalog::removeDir(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][REMOVEDIR]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::rename(const std::string& oldPath, const std::string& newPath) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][RENAME]" << std::endl;
  std::string tmp_oldPath = __sanitizePath(oldPath);
  std::string tmp_newPath = __sanitizePath(newPath);
}

void RucioCatalog::setAcl(const std::string& path, const dmlite::Acl& acl) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETACL]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setChecksum(const std::string& path, const std::string& csumtype, const std::string &
                               csumvalue) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETCHECKSUM]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setComment(const std::string& path, const std::string& comment) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETCOMMENT]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setGuid(const std::string& path, const std::string& guid) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETGUID]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setMode(const std::string& path, mode_t mode) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETMODE]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setOwner(const std::string& path, uid_t newUid, gid_t newGid,
                            bool followSymlink) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETOWNER]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setSecurityContext(const dmlite::SecurityContext *ctx) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETSECURITYCONTEXT]" << std::endl;
}

void RucioCatalog::setSize(const std::string& path, size_t newSize) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETSIZE]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::setStackInstance(dmlite::StackInstance *si) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SETSTACKINSTANCE]" << std::endl;
}

void RucioCatalog::symlink(const std::string& path, const std::string symlink) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][SYMLINK]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

mode_t RucioCatalog::umask(mode_t mask) throw () {
  std::cerr << "[RUCIO][CATALOG][UMASK]" << std::endl;
  return 0;
}

void RucioCatalog::unlink(const std::string& path) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UNLINK]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::updateExtendedAttributes(const std::string& path, const dmlite::Extensible &
                                            attr) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UPDATEEXTENDEDATTRIBUTES]" << std::endl;
  std::string tmp_path = __sanitizePath(path);
}

void RucioCatalog::updateReplica(const dmlite::Replica& replica) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UPDATEREPLICA]" << std::endl;
}

void RucioCatalog::utime(const std::string& path, const struct utimbuf *buf) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][CATALOG][UTIME]" << std::endl;

  std::string tmp_path = __sanitizePath(path);
}

std::string RucioCatalog::__sanitizePath(std::string path) {

  std::string tmp_path;

  if (path == ".") {
    tmp_path = "/";
    for (uint i = 0; i < cwd.size(); ++i) {
      tmp_path = cwd.at(i) + "/";
    }
    if (tmp_path != "/") {
      tmp_path = tmp_path.substr(0, tmp_path.size() - 1);
    }
    return tmp_path;
  }

  tmp_path = path;
  size_t f;

  while ((f = tmp_path.find("//")) != std::string::npos) { // Remove double slashes
    tmp_path.replace(f, 2, std::string("/"));
  }

  if ((tmp_path.size() > 1) && (tmp_path.at(tmp_path.size() - 1) == '/')) { // Remove slash at the end
    tmp_path.erase(tmp_path.size() - 1);
  }

  return tmp_path;
}

std::deque<std::string> RucioCatalog::__splitPath(std::string path) {
  std::deque<std::string> tmp_split;

  size_t prev = 0;
  size_t next = 0;

  while ((next = path.find_first_of("/", prev)) != std::string::npos) {
    if ((next - prev) != 0) {
      tmp_split.push_back(path.substr(prev, next - prev));
    }
    prev = next + 1;
  }

  if (prev < path.size()) {
    tmp_split.push_back(path.substr(prev));
  }

  return tmp_split;
}
}
