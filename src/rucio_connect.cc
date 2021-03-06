/**
 * Copyright European Organization for Nuclear Research (CERN)
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Authors:
 * - Mario Lassnig, <mario.lassnig@cern.ch>, 2012
 * - Daan van Dongen, <Daanvdongen@gmail.com>, 2013
 */

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <curl/curl.h>
#include <json/json.h>

#include "rucio_connect.h"

/* CURL callback */
size_t write_fp(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t actual_size = size * nmemb;
  mem_t *mem = (mem_t *)stream;

  mem->memory = (char *)realloc(mem->memory, mem->size + actual_size + 1);
  if (mem->memory == NULL) {
    std::cerr << "out of memory, cannot allocate " << mem->size + actual_size + 1 << " bytes";
    return 0;
  }

  memmove(&(mem->memory[mem->size]), ptr, actual_size);
  mem->size += actual_size;
  mem->memory[mem->size] = 0;
  return actual_size;
}

void clean_json_stream(mem_t *mem){ 				//cleans up the JSON stream to correctly turn in into a JSON-Array
	int apo_c = -1;						//Apostrophe counter
	bool check1 = TRUE;					//Check if the stream is just one big array
	int bra_c=0;						//Bracket counter

	bool testbool = FALSE;					//this bool is FALSE if it equals teststring
	std::string teststring= "DataIdentifierNotFound";
	for( int i=0 ; i<teststring.size() && testbool == FALSE && i<mem->size ; i++){
		if(mem->memory[i]!=teststring.at(i) ){
			testbool= TRUE;
		}
	}
	if( testbool == TRUE){
		testbool = FALSE;
		teststring= "None";
		for( int i=0 ; i<teststring.size() && testbool == FALSE && i<mem->size ; i++){
			if(mem->memory[i]!=teststring.at(i) ){
				testbool= TRUE;
			}
		}
	}

	if( testbool == TRUE){
		for( int i=0 ; i< mem->size ; i++){
			while(mem->memory[i]==' ' && i< mem->size){
				i++;
			}
			if( mem->memory[i]=='"'){
				apo_c = -1*apo_c;
			}
			if( apo_c == -1){				//check if you're not inside " "
				if( mem->memory[i] == '[' && check1 == TRUE){
					bra_c++;
				}
				if( mem->memory[i] == ']' && check1 == TRUE){
					bra_c--;
				}
				if( bra_c <= 0 && check1 == TRUE && i< mem->size-1){ 	//Also returns FALSE if (mem->memory[0]!='[')
					check1 = FALSE;
				}
				if( mem->memory[i]=='\n'){
					mem->memory[i]=',';
				}
				if( mem->memory[i]== '}' && i!=mem->size-1){	//Have not encountered these yet. But else this method would fail if you do.
					if( mem->memory[i+1]=='{'){
						std::cerr << "ERROR: Encountered \"}{\"" << std::endl;
					}
					if( mem->memory[i+1]=='['){
						std::cerr << "ERROR: Encountered \"}[\"" << std::endl;
					}
				}
				if( mem->memory[i]==']' && i!=mem->size-1){
					if( mem->memory[i+1]=='['){
						std::cerr << "ERROR: Encountered \"][\"" << std::endl;
					}
					if( mem->memory[i+1]=='{'){
						std::cerr << "ERROR: Encountered \"]{\"" << std::endl;
					}
				}
			}
		}

		if( check1 == FALSE ){
			char* tmp_c = new char[mem->size+2];
			tmp_c[0]='[';
			for( int i=0 ; i<mem->size ; i++){
				tmp_c[i+1] = mem->memory[i];
			}
			if( tmp_c[mem->size] == ',' ){
				tmp_c[mem->size]= ']';
				tmp_c[mem->size+1]= ' ';
			} else{
				tmp_c[mem->size+1]= ']';
			}
			delete[] mem->memory;
			mem->size += 2;
			mem->memory = tmp_c;
		}
	}
	/*if( mem->size == 0 || mem->size == 4){					//Dealing with empty strings
		char* tmp_cc = new char[2];
		tmp_cc[0] = '[';
		tmp_cc[1] = ']';
		//delete[] mem->memory;
		//free(mem->memory);
		mem-> size = 2;
		mem->memory = tmp_cc;
	}*/
}

std::deque<std::string> get_json_rsekeys(std::string json){
	std::deque<std::string> result;
	std::string tmp = "";
	bool atleastonce = FALSE;
	for(int i=0 ;  atleastonce == FALSE && i<json.size();i++){
		if(json.at(i) == '"'){
			atleastonce = TRUE;
		}
	}
	if(atleastonce == TRUE){
		int record = -1;
		for(int i=0 ; i<json.size() ; i++){
			while( i<json.size()-1 && json.at(i)== '\\'){
				i++;
			}
			if(record ==1 && json.at(i) != '"'){
				tmp = tmp + json.at(i);
			}
			if( json.at(i) == '"'){
				if(record ==1){
					result.push_back(tmp);
					tmp = "";
				}
				record = -1*record;
			}
		}
	} else {
		result.push_back(json);
	  }
	return result;
}

namespace Rucio {

RucioConnect::RucioConnect(std::string host, std::string port, std::string auth_token, std::string ca_cert) {

  full_host = "https://" + host + ":" + port;
  full_auth = std::string("X-Rucio-Auth-Token: ") + auth_token;

  headers = NULL;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_fp);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "plugin_rucio/0.1");
  curl_easy_setopt(curl, CURLOPT_CAINFO, ca_cert.c_str());
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);

  headers = curl_slist_append(headers, full_auth.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
}

RucioConnect::~RucioConnect() {
  curl_easy_cleanup(curl);
  curl_global_cleanup();
}

json_object *RucioConnect::http_get_url_json(std::string url) {
  std::cerr << "[CURL] " << url << std::endl;
  json_object *tmp_j = NULL;
  chunk.memory = (char *)malloc(1);
  chunk.size = 0;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  CURLcode res = curl_easy_perform(curl);
  bool testbool = FALSE;
  std::string teststring= "None";
  for( int i=0 ; i< teststring.size() && testbool == FALSE && i<chunk.size && i<teststring.size() ; i++){
	if(chunk.memory[i]!=teststring.at(i) ){
		testbool= TRUE;
	}
  }
  if (res != CURLE_OK || testbool == FALSE) {
	std::cerr << "curl_easy_perform failed(with certificate): " << curl_easy_strerror(res) << std::endl;
	free(chunk.memory);
	chunk.size = 0;
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
	res = curl_easy_perform(curl);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, TRUE);
	testbool = FALSE;
	teststring= "None";
	for( int i=0 ; i< teststring.size() && testbool == FALSE && i<chunk.size && i<teststring.size() ; i++){
		if(chunk.memory[i]!=teststring.at(i) ){
			testbool= TRUE;
		}
	}
	if (res != CURLE_OK || testbool == FALSE) {
		std::cerr << "curl_easy_perform failed(without): " << curl_easy_strerror(res) << std::endl;
	}
  }

  clean_json_stream(&chunk);
  std::cerr << "Chunk: " << chunk.memory << std::endl;

  tmp_j = json_tokener_parse(chunk.memory);
  if (!tmp_j) {
    std::cerr << "cannot parse json: " << chunk.memory << std::endl;
  }
  free(chunk.memory);
  return tmp_j;
}

std::deque<std::string> RucioConnect::list_scopes() {
  std::deque<std::string> response;
  json_object *tmp_j;
  tmp_j = http_get_url_json(full_host + "/scopes/");
  for (i = 0; i < json_object_array_length(tmp_j); ++i) {
    response.push_back(json_object_get_string(json_object_array_get_idx(tmp_j, i)));
  }
  json_object_put(tmp_j);
  return response;
}

std::deque<did_t> RucioConnect::list_dids(std::string scope, std::string did) {
  std::deque<did_t> response;

  json_object *tmp_j;
  if (did.empty()) {
    tmp_j = http_get_url_json(full_host + "/dids/" + scope + "/");
  } else {
    tmp_j = http_get_url_json(full_host + "/dids/" + scope + "/" + did + "/dids");
  }

  did_t tmp_did;
  for (i = 0; i < json_object_array_length(tmp_j); ++i) {
    json_object *tmp_jj = json_object_array_get_idx(tmp_j, i);
    tmp_did.name = json_object_get_string(json_object_object_get(tmp_jj, "name"));
    tmp_did.scope = json_object_get_string(json_object_object_get(tmp_jj, "scope"));
    tmp_did.type = json_object_get_string(json_object_object_get(tmp_jj, "type"));
    tmp_did.RSE = "";
    json_object_put(tmp_jj);
    response.push_back(tmp_did);
  }
  return response;
}

std::deque<did_t> RucioConnect::list_rses(std::string scope, std::string did) {
  std::deque<did_t> response;

  json_object *tmp_j;
  if (did.empty()) {
	tmp_j = NULL;
	std::cerr << "ERROR: trying to get rses without did" << std::endl;
  } else {
	tmp_j = http_get_url_json(full_host + "/dids/" + scope + "/" + did + "/rses");
  }

  did_t tmp_did;
  for (i = 0; i < json_object_array_length(tmp_j); ++i) {
    json_object *tmp_jj = json_object_array_get_idx(tmp_j, i);
    tmp_did.name = json_object_get_string(json_object_object_get(tmp_jj, "name"));
    tmp_did.RSE = json_object_get_string(json_object_object_get(tmp_jj, "rse"));
    tmp_did.scope = json_object_get_string(json_object_object_get(tmp_jj, "scope"));
    tmp_did.type = "RSE";
    tmp_did.RSE = json_object_get_string(json_object_object_get(tmp_jj, "rse"));
    json_object_put(tmp_jj);
    response.push_back(tmp_did);
  }

  return response;
}

std::deque<replica_t> RucioConnect::list_replicas(std::string scope, std::string did) {
  json_object *tmp_j;
  tmp_j = http_get_url_json(full_host + "/dids/" + scope + "/" + did + "/rses");

  std::deque<replica_t> response;
  replica_t tmp_repl;
  if(json_object_array_length(tmp_j) > 0){
	std::string str_rses =json_object_get_string( json_object_object_get(json_object_array_get_idx(tmp_j, 0) , "rses" ) );
	std::deque<std::string> deq_rses;
	if( str_rses != "" ){
		deq_rses = get_json_rsekeys(str_rses);
	} else {
		std::cerr << "ERROR: No rses to report" << std::endl;
	}

	json_object *tmp_jj = json_object_array_get_idx(tmp_j, 0);
	tmp_repl.pfn = "";
	tmp_repl.size = json_object_get_int(json_object_object_get(tmp_jj, "bytes"));
	tmp_repl.checksum = "0";
	for (i = 0; i < deq_rses.size(); ++i) {
		tmp_repl.RSE = deq_rses.at(i);
		response.push_back(tmp_repl);
	}
	json_object_put(tmp_jj);
  }
  return response;
}

did_t RucioConnect::get_did(std::string scope, std::string did) {
  json_object *tmp_j = http_get_url_json(full_host + "/dids/" + scope + "/" + did);
  did_t tmp_did;
  if (tmp_j != NULL) {
    json_object *tmp_jj = json_object_array_get_idx(tmp_j, 0);
    tmp_did.name = json_object_get_string(json_object_object_get(tmp_jj, "name"));
    tmp_did.scope = json_object_get_string(json_object_object_get(tmp_jj, "scope"));
    tmp_did.type = json_object_get_string(json_object_object_get(tmp_jj, "type"));
    tmp_did.RSE = "";
  }
  json_object_put(tmp_j);
  return tmp_did;
}

did_t RucioConnect::get_did_status(std::string scope, std::string did) {
  json_object *tmp_j;
  if( did.size() != 0) {
	tmp_j = http_get_url_json(full_host + "/dids/" + scope + "/" + did + "/status");
  } else {
	std::cerr << "ERROR: No did given" << std::endl;
  }
  did_t tmp_did;
  if (tmp_j != NULL) {
    json_object *tmp_jj = json_object_array_get_idx(tmp_j, 0);
    tmp_did.name = json_object_get_string(json_object_object_get(tmp_jj, "name"));
    tmp_did.scope = json_object_get_string(json_object_object_get(tmp_jj, "scope"));
    tmp_did.type = json_object_get_string(json_object_object_get(tmp_jj, "type"));
    tmp_did.RSE = "";
  }
  json_object_put(tmp_j);
  return tmp_did;
}

did_t RucioConnect::get_rse_status(std::string rse) {
  json_object *tmp_j;
  tmp_j = http_get_url_json(full_host + "/rses/" + rse );
  did_t tmp_did;
  if (tmp_j != NULL) {
    json_object *tmp_jj = json_object_array_get_idx(tmp_j, 0);
    tmp_did.name = json_object_get_string(json_object_object_get(tmp_jj, "rse"));
    tmp_did.scope = "";
    tmp_did.type = json_object_get_string(json_object_object_get(tmp_jj, "type"));
    tmp_did.RSE = json_object_get_string(json_object_object_get(tmp_jj, "rse"));
  }
  json_object_put(tmp_j);
  return tmp_did;
}
}
