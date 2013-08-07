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

#ifndef PLUGIN_RUCIO_H
#define PLUGIN_RUCIO_H

#include <string>

#include <dmlite/cpp/catalog.h>
#include <dmlite/cpp/dmlite.h>

#include <dmlite/cpp/poolmanager.h>
#include "rucio_connect.h"

namespace Rucio {

//Factory

class RucioFactory : public dmlite::CatalogFactory, public dmlite::PoolManagerFactory , public dmlite::PoolDriverFactory {
  public:
    RucioFactory(dmlite::CatalogFactory *next);
    RucioFactory(dmlite::PoolManagerFactory *next);
    RucioFactory(dmlite::PoolDriverFactory *next);
    ~RucioFactory();

    void configure(const std::string& key, const std::string& value) throw (dmlite::DmException);

    dmlite::Catalog *createCatalog(dmlite::PluginManager *pm) throw (dmlite::DmException);
    dmlite::PoolManager* createPoolManager(dmlite::PluginManager* pm) throw (dmlite::DmException);
    dmlite::PoolDriver* createPoolDriver() throw (dmlite::DmException);

    std::string implementedPool( ) throw ();

    dmlite::CatalogFactory *Cnext;
    dmlite::PoolManagerFactory* Pnext;
    dmlite::PoolDriverFactory* Dnext;

  private:
    std::string auth_host;
    std::string auth_method;
    std::string auth_token;
    std::string host;
    std::string port;
    std::string ca_cert;

};

//PoolManager

class RucioPoolManager: public dmlite::PoolManager{
 public:
  RucioPoolManager(Rucio::RucioFactory* factory, std::string host, std::string port, std::string auth_token, std::string ca_cert) throw (dmlite::DmException);
  ~RucioPoolManager();

  dmlite::Location whereToRead(const std::string& path) throw (dmlite::DmException);
  dmlite::Location whereToRead(ino_t inode) throw (dmlite::DmException);

  std::string getImplId() const throw();
  void setStackInstance(dmlite::StackInstance* SI) throw (dmlite::DmException);
  void setSecurityContext(const dmlite::SecurityContext* SC) throw (dmlite::DmException);
  std::vector<dmlite::Pool> getPools(PoolAvailability availability) throw (dmlite::DmException);
  dmlite:: Pool getPool(const std::string& poolname) throw (dmlite::DmException);
  void newPool(const dmlite::Pool& pool) throw (dmlite::DmException);
  void updatePool(const dmlite::Pool& pool) throw (dmlite::DmException);
  void deletePool(const dmlite::Pool& pool) throw (dmlite::DmException);
  dmlite::Location whereToWrite(const std::string& path) throw (dmlite::DmException);

 private:
  Rucio::RucioFactory* _factory;
  dmlite::PoolManagerFactory* next;
  dmlite::StackInstance* stack_;
  Rucio::RucioConnect *rc;
};

//PoolDriver

class RucioPoolDriver: public dmlite::PoolDriver{
 public:
  RucioPoolDriver()throw (dmlite::DmException);
  ~RucioPoolDriver();
  dmlite::PoolHandler* createPoolHandler(const std::string& name) throw (dmlite::DmException);
  dmlite::PoolHandler* getPoolHandler(const std::string& name) throw (dmlite::DmException);
  std::string implementedPool( ) throw ();
  std::string getImplId() const throw();
  void setStackInstance(dmlite::StackInstance* SI) throw (dmlite::DmException);
  void setSecurityContext(const dmlite::SecurityContext* SC) throw (dmlite::DmException);

 private:
  dmlite::StackInstance* stack_;
};

//PoolHandler

class RucioPoolHandler: public dmlite::PoolHandler{
 public:
  RucioPoolHandler()throw (dmlite::DmException);
  ~RucioPoolHandler();
  dmlite::Location whereToRead(const dmlite::Replica& repl) throw (dmlite::DmException);
};
}

std::string __sanitizePath(std::string path);

#endif
