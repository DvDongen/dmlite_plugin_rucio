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
