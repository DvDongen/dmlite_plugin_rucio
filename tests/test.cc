#include <iostream>

#include <dmlite/cpp/authn.h>
#include <dmlite/cpp/catalog.h>
#include <dmlite/cpp/dmlite.h>

int main(int argc, char *argv[]) {
  dmlite::PluginManager manager;
  dmlite::Catalog *catalog;
  dmlite::Directory *dir;
  dmlite::ExtendedStat *xstat;

  if (argc != 2) {
    std::cerr << "path missing" << '\n';
    return 1;
  }

  try {
    manager.loadConfiguration("/etc/dmlite.conf");
  } catch (dmlite::DmException& e) {
    std::cerr << e.what() << '\n';
    return e.code();
  }

  dmlite::StackInstance stack(&manager);

  dmlite::SecurityCredentials creds;
  creds.clientName = "/C=CH/O=CERN/OU=GD/CN=Test user 1";
  creds.remoteAddress = "127.0.0.1";

  try {
    stack.setSecurityCredentials(creds);
  } catch (dmlite::DmException& e) {
    std::cerr << e.what() << '\n';
    return e.code();
  }

  catalog = stack.getCatalog();

  try {
    std::cout << "should be /: " << catalog->getWorkingDir() << '\n';
  } catch (dmlite::DmException& e) {
    std::cerr << e.what() << '\n';
    return e.code();
  }

  try {
    catalog->changeDir(argv[1]);
  } catch (dmlite::DmException& e) {
    std::cerr << e.what() << '\n';
    return e.code();
  }

  try {
    std::cout << "should be " << argv[1] << ": " << catalog->getWorkingDir() << '\n';
  } catch (dmlite::DmException& e) {
    std::cerr << e.what() << '\n';
    return e.code();
  }

  try {
    dir = catalog->openDir(argv[1]);
    while ((xstat = catalog->readDirx(dir)) != NULL) {
      std::cout << '\t' << xstat->name << std::endl;
    }
    catalog->closeDir(dir);
  } catch (dmlite::DmException& e) {
    std::cerr << e.what() << std::endl;
    return e.code();
  }

  return 0;
}
