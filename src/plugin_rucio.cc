#include <iostream>
#include <string>

#include <dmlite/cpp/catalog.h>
#include <dmlite/cpp/dmlite.h>

#include "plugin_rucio.h"
#include "rucio_catalog.h"

namespace Rucio {

RucioFactory::RucioFactory(dmlite::CatalogFactory *next) : next(next) {
  host = "atlas-rucio.cern.ch";
  auth_host = "atlas-rucio-auth.cern.ch";
  auth_method = "userpass";
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
