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

#include <fstream>
#include <iostream>
#include <string>

#include <dmlite/cpp/catalog.h>
#include <dmlite/cpp/dmlite.h>

#include "plugin_rucio.h"
#include "rucio_catalog.h"

namespace Rucio {

//Factory

RucioFactory::RucioFactory(dmlite::CatalogFactory *next) : Cnext(next) {
  host = "fallback-hostname.com";
  port = "443";
  auth_host = "fallback-hostname.com";
  auth_method = "userpass";
  ca_cert = "/opt/rucio/etc/web/ca.crt";
}

RucioFactory::RucioFactory(dmlite::PoolManagerFactory *next) : Pnext(next) {
  host = "fallback-hostname.com";
  port = "443";
  auth_host = "fallback-hostname.com";
  auth_method = "userpass";
  ca_cert = "/opt/rucio/etc/web/ca.crt";
}

RucioFactory::RucioFactory(dmlite::PoolDriverFactory *next) : Dnext(next) {
  host = "fallback-hostname.com";
  port = "443";
  auth_host = "fallback-hostname.com";
  auth_method = "userpass";
  ca_cert = "/opt/rucio/etc/web/ca.crt";
}

RucioFactory::~RucioFactory() {
}

void RucioFactory::configure(const std::string& key, const std::string& value) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][FACTORY][CONFIGURE]" << key << " = " << value << std::endl;

  if (key == "RucioAuthHost") {
    this->auth_host = value;
  } else if (key == "RucioAuthMethod") {
    this->auth_method = value;
  } else if (key == "RucioHost") {
    this->host = value;
  } else if (key == "RucioPort") {
    this->port = port;
  } else if (key == "RucioAuthToken") {
    this->auth_token = value;
  } else if (key == "RucioCACert") {
    this->ca_cert = value;
  } else {
    throw dmlite::DmException(DMLITE_CFGERR(DMLITE_UNKNOWN_KEY), "unknown key: " + key);
  }
}

dmlite::Catalog* RucioFactory::createCatalog(dmlite::PluginManager *pm) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][FACTORY][CREATECATALOG]" << std::endl;
  // Check that we can actually access the CA certificate, otherwise CURL will fail later in the process.
  std::ifstream ca_file(ca_cert.c_str(), std::ifstream::in);
  if (!ca_file.good()) {
    ca_file.close();
    throw dmlite::DmException(DMLITE_CFGERR(EINVAL), "cannot open CA certificate: " + ca_cert);
  }
  ca_file.close();

  return new RucioCatalog(dmlite::CatalogFactory::createCatalog(Cnext, pm), host, port, auth_token, ca_cert);
}

dmlite::PoolManager* RucioFactory::createPoolManager(dmlite::PluginManager* pm) throw (dmlite::DmException){
  std::cerr << "[RUCIO][FACTORY][CREATEPOOLMANAGER]" << std::endl;
  // Check that we can actually access the CA certificate, otherwise CURL will fail later in the process.
  std::ifstream ca_file(ca_cert.c_str(), std::ifstream::in);
  if (!ca_file.good()) {
    ca_file.close();
    throw dmlite::DmException(DMLITE_CFGERR(EINVAL), "cannot open CA certificate: " + ca_cert);
  }
  ca_file.close();

  RucioPoolManager* response=NULL;
  response = new RucioPoolManager(this, host,port, auth_token, ca_cert);
  return response;
}

dmlite::PoolDriver* RucioFactory::createPoolDriver() throw (dmlite::DmException){
  std::cerr << "[RUCIO][FACTORY][CREATEPOOLDRIVER]" << std::endl;
  // Check that we can actually access the CA certificate, otherwise CURL will fail later in the process.
  std::ifstream ca_file(ca_cert.c_str(), std::ifstream::in);
  if (!ca_file.good()) {
    ca_file.close();
    throw dmlite::DmException(DMLITE_CFGERR(EINVAL), "cannot open CA certificate: " + ca_cert);
  }
  ca_file.close();

  RucioPoolDriver* response=NULL;
  response = new RucioPoolDriver();
  return response;
}

std::string RucioFactory::implementedPool( ) throw (){
  std::cerr << "[RUCIO][FACTORY][IMPLEMENTEDPOOL]" <<std::endl<<std::endl;
  return "metalink";
}

//POOLMANAGER

RucioPoolManager::RucioPoolManager(Rucio::RucioFactory* factory, std::string host, std::string port, std::string auth_token, std::string ca_cert) throw (dmlite::DmException): _factory(factory) , next(factory->Pnext){
  std::cerr << "[RUCIO][POOLMANAGER][CTOR]" << std::endl;
  rc = new Rucio::RucioConnect(host, port , auth_token, ca_cert);
  std::cerr << "https://" <<  host << ":" << port << std::endl;
}

RucioPoolManager::~RucioPoolManager(){
	std::cerr << "[RUCIO][POOLMANAGER][DTOR]" << std::endl;
	delete rc;
}

dmlite::Location RucioPoolManager::whereToRead(const std::string& path) throw (dmlite::DmException){	//redirect in cases without davfs
  std::cerr << "[RUCIO][POOLMANAGER][WHERETOREAD][PATH]" << std::endl;
  dmlite::Location where;
  dmlite::Chunk myChunk;
  std::string tmp_path = __sanitizePath(path);
  std::cerr << tmp_path << std::endl;
  tmp_path = tmp_path + "?metalink";
  myChunk.url = dmlite::Url("localhost" + tmp_path);
  where.push_back(myChunk);
  return where;
}


dmlite::Location RucioPoolManager::whereToRead(ino_t inode) throw (dmlite::DmException){
  std::cerr << "[RUCIO][POOLMANAGER][WHERETOREAD][INODE]" << std::endl;
  dmlite::Location where;
  return where;
}

std::string RucioPoolManager::getImplId() const throw (){
  std::cerr << std::endl << "[RUCIO][POOLMANAGER][GETIMPLID]" <<std::endl<<std::endl;
  return "metalink";
}


void RucioPoolManager::setStackInstance(dmlite::StackInstance* SI) throw (dmlite::DmException){
  std::cerr  << "[RUCIO][POOLMANAGER][STACKINSTANCE]" << std::endl;
  this->stack_ = SI;
}
void RucioPoolManager::setSecurityContext(const dmlite::SecurityContext* SC) throw (dmlite::DmException){
  std::cerr << "[RUCIO][POOLMANAGER][SECURITYCONTEXT]" << std::endl;
}


std::vector<dmlite::Pool> RucioPoolManager::getPools(PoolAvailability availability) throw (dmlite::DmException){
  std::cerr  << "[RUCIO][POOLMANAGER][POOLAVAILABILITY]" << std::endl;
}

dmlite::Pool RucioPoolManager::getPool(const std::string& poolname) throw (dmlite::DmException){		//Being called to look for a handler for metalinks
  std::cerr  << "[RUCIO][POOLMANAGER][GETPOOL]" << std::endl;
  std::cerr  << poolname << std::endl;
  dmlite::Pool response;
  response.name = "metalink";
  response.type = "metalink";
  return response;
}

void RucioPoolManager::newPool(const dmlite::Pool& pool) throw (dmlite::DmException){
  std::cerr <<  "[RUCIO][POOLMANAGER][NEWPOOL]" << std::endl;
}

void RucioPoolManager::updatePool(const dmlite::Pool& pool) throw (dmlite::DmException){
  std::cerr  << "[RUCIO][POOLMANAGER][UPDATEPOOL]" << std::endl;
}
void RucioPoolManager::deletePool(const dmlite::Pool& pool) throw (dmlite::DmException){
  std::cerr  << "[RUCIO][POOLMANAGER][DELETEPOOL]" << std::endl;
}

dmlite::Location RucioPoolManager::whereToWrite(const std::string& path) throw (dmlite::DmException){
  std::cerr  << "[RUCIO][WHERETOWRITE]" << std::endl;
}

//POOLDRIVER

RucioPoolDriver::RucioPoolDriver() throw (dmlite::DmException){
  std::cerr << "[RUCIO][POOLDRIVER][CTOR]" << std::endl;
}

RucioPoolDriver::~RucioPoolDriver(){
  std::cerr << "[RUCIO][POOLDRIVER][DTOR]" << std::endl;
}

dmlite::PoolHandler* RucioPoolDriver::getPoolHandler(const std::string& name) throw (dmlite::DmException){
  std::cerr << "[RUCIO][POOLDRIVER][GETPOOLHANDLER]" << std::endl;
  dmlite::PoolHandler* response = new RucioPoolHandler();
  return response;
}

dmlite::PoolHandler* RucioPoolDriver::createPoolHandler(const std::string& name) throw (dmlite::DmException){
  std::cerr << "[RUCIO][POOLDRIVER][CREATEPOOLHANDLER]" << std::endl;
  dmlite::PoolHandler* response = new RucioPoolHandler();
  return response;
}

std::string RucioPoolDriver::implementedPool() throw (){
  std::cerr  << "[RUCIO][POOLDRIVER][IMPLEMPOOL]" <<std::endl;
  return "metalink";
}

std::string RucioPoolDriver::getImplId() const throw (){
  std::cerr << "[RUCIO][POOLDRIVER][GETIMPLID]" <<std::endl<<std::endl;
  return "metalink";
}

void RucioPoolDriver::setStackInstance(dmlite::StackInstance* SI) throw (dmlite::DmException){
  std::cerr  << "[RUCIO][POOLDRIVER][STACKINSTANCE]" << std::endl;
  this->stack_ = SI;
}
void RucioPoolDriver::setSecurityContext(const dmlite::SecurityContext* SC) throw (dmlite::DmException){
  std::cerr << "[RUCIO][POOLDRIVER][SECURITYCONTEXT]" << std::endl;
}

//POOLHANDLER

RucioPoolHandler::RucioPoolHandler() throw (dmlite::DmException){
  std::cerr << "[RUCIO][POOLHANDLER][CTOR]" << std::endl;
}

RucioPoolHandler::~RucioPoolHandler(){
  std::cerr << "[RUCIO][POOLHANDLER][DTOR]" << std::endl;
}


dmlite::Location RucioPoolHandler::whereToRead(const dmlite::Replica& repl) throw (dmlite::DmException){	//Being called to write in the metalink file
  std::cerr << "[RUCIO][POOLHANDLER][WHERETOREAD]" << std::endl;
  dmlite::Location response;
  dmlite::Chunk myChunk;
  std::string tmp_path = __sanitizePath(repl.rfn);
  std::cerr << repl.server << "/" << tmp_path << std::endl;
  myChunk.url = dmlite::Url( repl.server +"/"+ tmp_path);
  response.push_back(myChunk);
  return response;
}
}


static void registerPluginRucio(dmlite::PluginManager *pm) throw (dmlite::DmException) {
  pm->registerCatalogFactory(new Rucio::RucioFactory(pm->getCatalogFactory()));
  pm->registerPoolManagerFactory(new Rucio::RucioFactory(pm->getPoolManagerFactory()));
  dmlite::PoolDriverFactory* pdf=NULL;
  pm->registerPoolDriverFactory(new Rucio::RucioFactory(pdf));
}

dmlite::PluginIdCard plugin_rucio = {
  PLUGIN_ID_HEADER,
  registerPluginRucio
};





std::string __sanitizePath(std::string path) {
  std::string tmp_path;
  if (path == ".") {
    std::cerr << "path = \".\"?" <<std::endl;
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
