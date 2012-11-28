/**
 * Copyright European Organization for Nuclear Research (CERN)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Authors:
 * - Mario Lassnig, <mario.lassnig@cern.ch>, 2012
 */

#include <iostream>
#include <string>

#include <dmlite/cpp/catalog.h>
#include <dmlite/cpp/dmlite.h>

#include "plugin_rucio.h"
#include "rucio_catalog.h"

namespace Rucio {

RucioFactory::RucioFactory(dmlite::CatalogFactory *next) : next(next) {
  host = "fallback-hostname.com";
  auth_host = "fallback-hostname.com";
  auth_method = "userpass"; // one of 'userpass', 'x509', 'x509_proxy', 'gss'
}

RucioFactory::~RucioFactory() {
}

void RucioFactory::configure(const std::string& key, const std::string& value) throw (dmlite::DmException) {
  std::cerr << "[RUCIO][FACTORY][CONFIGURE] " << key << " = " << value << std::endl;

  if (key == "RucioAuthHost") {
    this->auth_host = value;
  } else if (key == "RucioAuthMethod") {
    this->auth_method = value;
  } else if (key == "RucioHost") {
    this->host = value;
  } else if (key == "RucioAuthToken") {
    this->auth_token = value;
  } else {
    throw dmlite::DmException(DMLITE_CFGERR(DMLITE_UNKNOWN_KEY), "unknown key: " + key);
  }
}

dmlite::Catalog *RucioFactory::createCatalog(dmlite::PluginManager *pm) throw (dmlite::DmException) {
  return new RucioCatalog(dmlite::CatalogFactory::createCatalog(next, pm), host, auth_token);
}
}

static void registerPluginRucio(dmlite::PluginManager *pm) throw (dmlite::DmException) {
  pm->registerCatalogFactory(new Rucio::RucioFactory(pm->getCatalogFactory()));
}

dmlite::PluginIdCard plugin_rucio = {
  PLUGIN_ID_HEADER,
  registerPluginRucio
};
