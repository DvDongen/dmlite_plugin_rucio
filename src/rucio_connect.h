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

#ifndef RUCIO_CONNECT_H
#define RUCIO_CONNECT_H

#include <deque>
#include <string>

#include <curl/curl.h>
#include <json/json.h>

/* CURL callback */
typedef struct {
  char *memory;
  size_t size;
} mem_t;
size_t write_fp(void *ptr, size_t size, size_t nmemb, void *stream);

namespace Rucio {

typedef struct {
  std::string scope;
  std::string type;
  std::string name;
  std::string RSE;
} did_t;

typedef struct  {
  std::string rse;
  std::string pfn;
  std::string checksum;
  int size;
} replica_t;

class RucioConnect {

  public:
    RucioConnect(std::string host, std::string port, std::string auth_token, std::string ca_cert);
    ~RucioConnect();

    std::deque<std::string> list_scopes();

    std::deque<did_t> list_dids(std::string scope, std::string did);

    std::deque<replica_t> list_replicas(std::string scope, std::string did);

    did_t get_did(std::string scope, std::string did);

    std::deque<did_t> list_rses(std::string scope, std::string did);

    did_t get_did_status(std::string scope, std::string did);

    did_t get_rse_status(std::string rse);

  private:
    struct curl_slist *headers;

    mem_t chunk;
    CURL *curl;

    unsigned int i;

    std::string full_host;
    std::string full_auth;

    json_object *http_get_url_json(std::string url);
};
}

#endif
