/**
 * Copyright European Organization for Nuclear Research (CERN)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Authors:
 * - Mario Lassnig, <mario.lassnig@cern.ch>, 2012
 */

#include <cstring>
#include <iostream>

#include "jansson.h"

#include "rucio_connect.h"

/* CURL callback */
size_t write_fp(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t actual_size = size * nmemb;
  mem_t *mem = (mem_t *)stream;

  mem->memory = (char *)realloc(mem->memory, mem->size + actual_size + 1);
  if (mem->memory == NULL) {
    std::cerr << "out of memory";
    return 0;
  }

  memmove(&(mem->memory[mem->size]), ptr, actual_size);
  mem->size += actual_size;
  mem->memory[mem->size] = 0;

  return actual_size;
}

namespace Rucio {

RucioConnect::RucioConnect(std::string host, std::string auth_token) {

  full_host = "https://" + host + ":443";
  full_auth = std::string("Rucio-Auth-Token: ") + auth_token;

  headers = NULL;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fp);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "plugin_rucio/0.1");
  curl_easy_setopt(curl, CURLOPT_CAINFO, "/opt/rucio/etc/web/ca.crt");

  headers = curl_slist_append(headers, full_auth.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
}

RucioConnect::~RucioConnect() {
  curl_easy_cleanup(curl);
  curl_global_cleanup();
}

json_t *RucioConnect::http_get_json(std::string url) {
  json_t *tmp_j;
  json_error_t json_error;

  chunk.memory = (char *)malloc(1);
  chunk.size = 0;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_perform(curl);
  tmp_j = json_loads(chunk.memory, 0, &json_error);
  if (!tmp_j) {
    std::cerr << "invalid json response from server: " << json_error.line << " -- " << json_error.text << std::endl;
  }
  free(chunk.memory);

  return tmp_j;
}

std::deque<std::string> RucioConnect::list_scopes() {
  std::deque<std::string> response;

  json_t *tmp_j = http_get_json(full_host + "/scopes/");
  for (i = 0; i < json_array_size(tmp_j); ++i) {
    response.push_back(json_string_value(json_array_get(tmp_j, i)));
  }
  json_decref(tmp_j);

  return response;
}

std::deque<did_t> RucioConnect::list_dids(std::string scope, std::string did) {
  std::deque<did_t> response;

  json_t *tmp_j;

  if (did.empty()) {
    tmp_j = http_get_json(full_host + "/dids/" + scope + "/");
  } else {
    tmp_j = http_get_json(full_host + "/dids/" + scope + "/" + did + "/dids");
  }

  for (i = 0; i < json_array_size(tmp_j); ++i) {
    json_t *tmp_jj = json_array_get(tmp_j, i);
    did_t tmp_did;
    tmp_did.did = json_string_value(json_object_get(tmp_jj, "did"));
    tmp_did.scope = json_string_value(json_object_get(tmp_jj, "scope"));
    tmp_did.type = json_string_value(json_object_get(tmp_jj, "type"));
    response.push_back(tmp_did);
  }
  json_decref(tmp_j);

  return response;
}

std::deque<replica_t> RucioConnect::list_replicas(std::string scope, std::string did) {
  std::string url;
  http_get_json(url);

  std::deque<replica_t> dummy;
  return dummy;
}
}
