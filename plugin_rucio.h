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

    void configure(const std::string& key,
                   const std::string& value) throw (dmlite::DmException);

    dmlite::Catalog *createCatalog(dmlite::PluginManager *pm) throw (dmlite::
                                                                     DmException);

  private:
    std::string auth_host;
    std::string auth_method;
    std::string auth_token;
    std::string host;
    dmlite::CatalogFactory *next;
};
}
#endif
