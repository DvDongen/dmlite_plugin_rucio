/**
 * Copyright European Organization for Nuclear Research (CERN)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Authors:
 * - Mario Lassnig, <mario.lassnig@cern.ch>, 2012
 */

#ifndef PLUGIN_RUCIO_H
#define PLUGIN_RUCIO_H

#include <string>

#include <dmlite/cpp/catalog.h>
#include <dmlite/cpp/dmlite.h>

namespace Rucio {

class RucioFactory : public dmlite::CatalogFactory {
  public:
    RucioFactory(dmlite::CatalogFactory *next);
    ~RucioFactory();

    void configure(const std::string& key, const std::string& value) throw (dmlite::DmException);

    dmlite::Catalog *createCatalog(dmlite::PluginManager *pm) throw (dmlite::DmException);

  private:
    std::string auth_host;
    std::string auth_method;
    std::string auth_token;
    std::string host;
    std::string ca_cert;
    dmlite::CatalogFactory *next;
};
}
#endif
